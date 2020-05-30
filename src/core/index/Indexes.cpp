#include "Indexes.h"
#include <unordered_map>
#include "rocapinyin.h"
#include "utility/Util.h"

namespace Skilo {

namespace Index{

using StorageService=Storage::StorageService;

InvertIndex::InvertIndex()
{

}

void InvertIndex::index_str_record(const IndexRecord &record)
{
    for(const auto &[term,offsets]:record.term_offsets){
        {
            WriterLockGuard lock_guard(this->_term_posting_lock);
            PostingList *posting_list=this->get_postinglist(term);
            if(!posting_list){
                posting_list=new PostingList(term);
                _term_postings.insert(term,posting_list);
            }
            posting_list->add_doc(record.seq_id,record.doc_len,offsets);
        }
        if(Util::is_chinese(term)){
            WriterLockGuard lock_guard(this->_pinyin_terms_lock);
            std::string pinyin=rocapinyin::getpinyin_str(term);
            Util::trim(pinyin,' ');
            std::unordered_set<std::string> *term_set=this->_pinyin_terms.find(pinyin);
            if(!term_set){
                term_set=new std::unordered_set<std::string>();
                _pinyin_terms.insert(pinyin,term_set);
            }
            term_set->emplace(term);
        }
    }
}

uint32_t InvertIndex::term_docs_num(const string &term) const
{
    ReaderLockGuard lock_guard(this->_term_posting_lock);
    PostingList *posting_list=this->get_postinglist(term);
    return posting_list?posting_list->num_docs():0;
}

PostingList *InvertIndex::get_postinglist(const string &term) const
{
    return _term_postings.find(term);
}

void InvertIndex::search_field(const std::string &field_path,const std::unordered_map<string,std::vector<uint32_t>> &token_to_offsets,
                               size_t slop,uint32_t total_doc_count,const std::unordered_map<std::string, SortIndex> *sort_indexes,std::function<void(Search::MatchContext&)> on_match) const
{
    /// \brief find the conjuction doc_ids of the query terms
    /// @example:
    /// query_term_A -> doc_id_1, doc_id_5, ...
    /// query_term_B -> doc_id_2, doc_id_4, doc_id_5...
    /// query_term_C -> doc_id_5, doc_id_7, doc_id_15, doc_id 29...
    /// conjuction result: doc_id_5 ...

    std::vector<const PostingList*> candidate_postings; //for term AND logic, none duplicate
    std::vector<std::pair<uint32_t,const PostingList*>> query_offset_entry; //<query term offset, term postings>, for phrase match

    ReaderLockGuard lock_guard(this->_term_posting_lock);
    for(const auto &[query_term,offsets]:token_to_offsets){
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
                        goto END_OF_MATCH;
                    }
                    term_cursors[i].first=cur_pos;
                    long term_rel=term_offs[cur_pos]-following_qt_offset;
                    if(term_rel>rel_pos){ //one of the following term's relative offset mismatch
                        next_rel_pos=term_rel;
                        break;
                    }
                }

                if(next_rel_pos>rel_pos+slop){ //handle mismatch
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
            Search::MatchContext context{lead_doc,total_doc_count,&field_path,&candidate_postings,phrase_match_count,&token_to_offsets,sort_indexes};
            on_match(context);
        }
END_OF_MATCH:
        void(0);
    }
}

void InvertIndex::iterate_terms(const string &prefix, std::function<void (unsigned char *, size_t, PostingList *)> on_term,
    std::function<bool(unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const
{
    ReaderLockGuard lock_guard(this->_term_posting_lock);
    _term_postings.iterate(prefix.data(),prefix.length(),on_term,early_termination,on_backtrace);
}

void InvertIndex::iterate_pinyin(const string &prefix, std::function<void (unsigned char *, size_t, std::unordered_set<string> *)> on_pinyin, std::function<bool (unsigned char)> early_termination, std::function<void(unsigned char)> on_backtrace) const
{
    ReaderLockGuard lock_guard(this->_pinyin_terms_lock);
    _pinyin_terms.iterate(prefix.data(),prefix.length(),on_pinyin,early_termination,on_backtrace);
}

size_t InvertIndex::dict_size() const
{
    ReaderLockGuard lock_guard(this->_term_posting_lock);
    return _term_postings.size();
}

void InvertIndex::debug_print_term_dict() const
{
    std::cout<<">>>>>>>term dict start:"<<std::endl;
    this->iterate_terms("",[](unsigned char *data,size_t len,PostingList *){
        std::cout<<data<<std::endl;
    },[](unsigned char){return false;},[](unsigned char){return;});
    std::cout<<"<<<<<<<<term dict end"<<std::endl;
}

CollectionIndexes::CollectionIndexes(const uint32_t collection_id, const Schema::CollectionSchema &schema,
                                     const CollectionMeta &collection_meta,const Storage::StorageService *storage_service)
    :_collection_id(collection_id),_storage_service(storage_service)
{
    schema.accept(*this); //create indexes according to fields in the schema
    if(collection_meta.enable_auto_suggestion()){
        uint32_t suggest_entry_num=collection_meta.get_suggest_entry_num(5);
        uint32_t min_gram=collection_meta.get_min_gram(2);
        uint32_t max_gram=collection_meta.get_max_gram(15);
        _auto_suggestor=std::make_unique<Search::AutoSuggestor>(suggest_entry_num,min_gram,max_gram);
    }
}

void CollectionIndexes::visit_field_string(const Schema::FieldString *field_string)
{
    if(field_string->attribute_val_true("index")){
        _indexes.emplace(field_string->path,InvertIndex());
    }
}

void CollectionIndexes::visit_field_integer(const Schema::FieldInteger *field_integer)
{
    init_sort_field(field_integer);
}

void CollectionIndexes::visit_field_float(const Schema::FieldFloat *field_float)
{
    init_sort_field(field_float);
}

void CollectionIndexes::init_sort_field(const Schema::Field *field_numeric)
{
    SortIndex sort_index;
    sort_index.collection_id=this->_collection_id;
    sort_index.field_path=field_numeric->path;
    sort_index.storage=const_cast<Storage::StorageService*>(this->_storage_service);
    sort_index.cache=field_numeric->attribute_val_true("sort_field");
    _sort_indexes.emplace(field_numeric->path,sort_index);
}

const InvertIndex *CollectionIndexes::get_invert_index(const string &field_path) const
{
    auto field_it=this->_indexes.find(field_path);
    return field_it!=_indexes.cend()?&field_it->second:nullptr;
}

Search::AutoSuggestor *CollectionIndexes::get_suggestor() const
{
    return _auto_suggestor.get();
}

InvertIndex *CollectionIndexes::get_invert_index(const string &field_path)
{
    return const_cast<InvertIndex*>(static_cast<const CollectionIndexes*>(this)->get_invert_index(field_path));
}

bool CollectionIndexes::contains(const string &field_path) const
{
    return _indexes.find(field_path)!=_indexes.end();
}

void CollectionIndexes::search_fields(const std::string &field_path,const std::unordered_map<string,std::vector<uint32_t>> &token_to_offsets,size_t slop,std::function<void(Search::MatchContext&)> on_match) const
{
    const InvertIndex *index=this->get_invert_index(field_path);
    if(!index)
        return;

    index->search_field(field_path,token_to_offsets,slop,_doc_count,&this->_sort_indexes,on_match);
}

void CollectionIndexes::index_numeric(const string &field_path, const uint32_t seq_id, const number_t number)
{
    SortIndex &sort_index=_sort_indexes[field_path];
    if(sort_index.cache){
        sort_index.add_number(seq_id,number);
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

number_t SortIndex::get_numeric_val(const uint32_t doc_seq_id) const
{
    auto num_it=index.find(doc_seq_id);
    if(num_it!=index.end()){
        return num_it->second;
    }
    //not in the index, load from storage service
    Document doc=storage->get_document(collection_id,doc_seq_id);
    rapidjson::Value &raw_value=doc.get_value(field_path);
    number_t value=raw_value.IsNumber()?raw_value.GetInt64():raw_value.GetDouble();
    index[doc_seq_id]=value; //load to index
    return value;
}

void SortIndex::add_number(uint32_t seq_id,const number_t number)
{
    index[seq_id]=number;
}


} //namespace Index
} //namespace Skilo
