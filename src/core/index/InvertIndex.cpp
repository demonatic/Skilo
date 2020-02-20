#include "InvertIndex.h"
#include <unordered_map>

namespace Skilo {
namespace Index{

InvertIndex::InvertIndex()
{

}

void InvertIndex::add_record(const IndexRecord &record)
{
    WriterLockGuard lock_guard(this->_index_lock);
    for(const auto &[term,offsets]:record.term_offsets){
        PostingList *posting_list=this->get_postinglist(term);
        if(!posting_list){
            posting_list=new PostingList();
            _index.insert(term.data(),term.length(),posting_list);
        }
        posting_list->add_doc(record.seq_id,offsets);
    }
}

uint32_t InvertIndex::term_docs_num(const string &term) const
{
    ReaderLockGuard lock_guard(this->_index_lock);
    PostingList *posting_list=this->get_postinglist(term);
    return posting_list?posting_list->num_docs():0;
}

PostingList *InvertIndex::get_postinglist(const string &term) const
{
    return _index.find(term.data(),term.length());
}

void InvertIndex::search_field(const std::unordered_map<string, std::vector<uint32_t>> &query_terms, const string &field_path,
                               Search::HitCollector &collector, uint32_t total_doc_count) const
{
    /// \brief find the conjuction doc_ids of the query terms
    /// @example:
    /// query_term_A -> doc_id_1, doc_id_5, ...
    /// query_term_B -> doc_id_2, doc_id_4, doc_id_5...
    /// query_term_C -> doc_id_5, doc_id_7, doc_id_15, doc_id 29...
    /// conjuction result: doc_id_5 ...

    std::vector<const PostingList*> candidate_postings; //for term AND logic, none duplicate
    std::vector<std::pair<uint32_t,const PostingList*>> query_offset_entry; //<query term offset, term postings>, for phrase match

    ReaderLockGuard lock_guard(this->_index_lock);
    for(const auto &[query_term,offsets]:query_terms){
        auto *entry=this->get_postinglist(query_term);
        if(!entry) return;
        candidate_postings.push_back(entry);
        for(const uint32_t &off:offsets){
            query_offset_entry.push_back({off,entry});
        }
    }

    size_t query_term_count=query_offset_entry.size();

    //sort the posting list based on length in ascending order reduces the number of comparisons
    std::sort(candidate_postings.begin(),candidate_postings.end(),[](const auto &e1,const auto &e2){
        return e1->num_docs()<e2->num_docs();
    });
    //sort query term according to offset in ascending order
    std::sort(query_offset_entry.begin(),query_offset_entry.end(),[](auto &p1,auto &p2){
        return p1.first<p2.first;
    });

    uint32_t leading_doc_num=candidate_postings[0]->num_docs();
    std::unique_ptr<uint32_t[]> leading_docs=candidate_postings[0]->get_all_doc_ids();

    for(uint32_t leading_cur=0;leading_cur<leading_doc_num;leading_cur++){
        bool exists_in_all_entry=true;
        uint32_t lead_doc=leading_docs[leading_cur];

        for(size_t i=1;i<candidate_postings.size();i++){
            if(!candidate_postings[i]->contain_doc(lead_doc)){ //couldn't find lead_doc in this entey
                exists_in_all_entry=false;
                break;
            }
        }
        /// \brief check whether the doc contains the query phrase and if contains, how many
        /// @example:
        /// query phrase including three term "A","B","C" and term offsets are: 0, 2, 4
        /// a doc's offsets of:
        ///  >  query_term_A -> 11, 26, ...
        ///  >  query_term_B -> 7, 13, 19, 28...
        ///  >  query_term_C -> 15, 21, 30, 100...
        ///
        /// @details: since 11-0=13-2=15-4, we got a phrase match the query phrase
        ///           since 26-0=28-2=30-4, we got another match...

        if(exists_in_all_entry){
            std::vector<std::pair<uint32_t,std::vector<uint32_t>>> term_cursors; //<posting cur, doc term offsets>
            for(std::pair<uint32_t,const PostingList*> pair:query_offset_entry){
                std::vector<uint32_t> doc_term_offsets=pair.second->get_doc_term_offsets(lead_doc);
                term_cursors.emplace_back(0,std::move(doc_term_offsets));
            }

            std::vector<uint32_t> &leading_term_offsets=term_cursors[0].second;
            uint32_t leading_qt_offset=query_offset_entry[0].first;
            uint32_t phrase_match_count=0;
            while(term_cursors[0].first<leading_term_offsets.size()){
                long rel_pos,next_rel_pos; //relative position
                rel_pos=next_rel_pos=leading_term_offsets[0]-leading_qt_offset; // equivalent to "11-0" in above example

                //move each term's cursor except the leading one to where the relative offset is no smaller than rel_pos
                for(size_t i=1;i<query_term_count;i++){
                    uint32_t cur_pos=term_cursors[i].first;
                    uint32_t following_qt_offset=query_offset_entry[i].first;
                    std::vector<uint32_t> &term_offs=term_cursors[i].second;

                    while(cur_pos<term_offs.size()&&term_offs[cur_pos]-following_qt_offset<rel_pos){ //skip '7' in term_B since 7-2<11-0 until reaching 13
                        cur_pos++;
                    }
                    if(cur_pos==term_offs.size()){ //one of the cursor go to the end
                        goto PHRASE_NOT_FOUND;
                    }
                    term_cursors[i].first=cur_pos;
                    long term_rel=term_offs[cur_pos]-following_qt_offset;
                    if(term_rel>rel_pos){ //one of the following term's relative offset mismatch
                        next_rel_pos=term_rel;
                        break;
                    }
                }

                if(next_rel_pos>rel_pos){ //handle mismatch
                    uint32_t leading_cur=term_cursors[0].first;
                    // move leading cursor A to next candidate position
                    while(leading_cur<leading_term_offsets.size()&&leading_term_offsets[leading_cur]-leading_qt_offset<next_rel_pos){
                        leading_cur++;
                    }
                    term_cursors[0].first=leading_cur;
                }
                else{ //all term's offset match, move cursor A to the right next position
                    phrase_match_count++;
                    term_cursors[0].first++;
                }
            }
            // collect this hit
            if(phrase_match_count>0){
                //TODO pass phrase count to hit context
                Search::HitContext context{lead_doc,total_doc_count,&field_path,&candidate_postings};
                collector.collect(context);
            }
        }
PHRASE_NOT_FOUND:
        void(0);
    }
}

size_t InvertIndex::dict_size() const
{
    ReaderLockGuard lock_guard(this->_index_lock);
    return _index.size();
}

CollectionIndexes::CollectionIndexes(const Schema::CollectionSchema &schema)
{
    schema.accept(*this); //create indexes according to fields in the schema
}

void CollectionIndexes::visit_field_string(const Schema::FieldString *field_string)
{
    auto it=field_string->attributes.find("index");
    if(it==field_string->attributes.end())
        return;

    const Schema::Field::ArrtibuteValue &index_option_value=it->second;
    if(std::get<bool>(index_option_value)){
        _indexes.emplace(field_string->path,InvertIndex());
    }
}

const InvertIndex *CollectionIndexes::get_index(const string &field_path) const
{
    auto field_it=this->_indexes.find(field_path);
    return field_it!=_indexes.cend()?&field_it->second:nullptr;
}

InvertIndex *CollectionIndexes::get_index(const string &field_path)
{
    return const_cast<InvertIndex*>(static_cast<const CollectionIndexes*>(this)->get_index(field_path));
}

bool CollectionIndexes::contains(const string &field_path) const
{
    return _indexes.find(field_path)!=_indexes.end();
}

void CollectionIndexes::search_fields(const std::unordered_map<string, std::vector<uint32_t>> &query_terms,
                                        const std::vector<string> &field_paths,Search::HitCollector &collector) const
{
    if(query_terms.empty())
        return;

    for(const std::string &path:field_paths){
        if(!this->contains(path)){
            std::string exist_fields;
            for(const auto &[exist_field_name,index]:_indexes){
                exist_fields.append("\""+exist_field_name+"\" ");
            }
            throw InvalidFormatException("field path \""+path+"\" is not found, exist fields: "+exist_fields);
        }
        const InvertIndex *index=this->get_index(path);
        if(index){
            index->search_field(query_terms,path,collector,_doc_count);
        }
    }
}

uint32_t CollectionIndexes::field_term_doc_num(const string &field_path, const string &term) const
{
    auto index_it=_indexes.find(field_path);
    if(index_it==_indexes.end()){
        return 0;
    }
    const InvertIndex &index=index_it->second;
    return index.term_docs_num(term);
}

void CollectionIndexes::set_doc_num(const uint32_t doc_num)
{
    _doc_count=doc_num;
}

uint32_t CollectionIndexes::get_doc_num() const
{
    return this->_doc_count;
}


} //namespace Index
} //namespace Skilo
