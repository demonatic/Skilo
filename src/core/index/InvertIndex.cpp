#include "InvertIndex.h"

namespace Skilo {
namespace Index{

InvertIndex::InvertIndex()
{

}

void InvertIndex::add_record(const IndexRecord &record)
{
    //TODO use offsets info if not empty
    for(const auto &[term,offsets]:record.term_offsets){
        PostingList *posting_list=this->get_postinglist(term);
        if(!posting_list){
            posting_list=new PostingList();
            _index.insert(term.data(),term.length(),posting_list);
        }
        cout<<"term="<<term<<" seq_id="<<record.seq_id<<" freq="<<offsets.size()<<std::endl;
        posting_list->add_doc(record.seq_id,offsets);
    }
}

uint32_t InvertIndex::num_docs(const string &term) const
{
    PostingList *posting_list=this->get_postinglist(term);
    return posting_list?posting_list->num_docs():0;
}

PostingList *InvertIndex::get_postinglist(const string &term) const
{
    return _index.find(term.data(),term.length());
}

size_t InvertIndex::dict_size() const
{
    return _index.size();
}

CollectionIndexes::CollectionIndexes(const Schema::CollectionSchema &schema)
{
    schema.accept(*this); //create indexes from fields in the schema
}

void CollectionIndexes::visit_field_string(const Schema::FieldString *field_string)
{
    auto it=field_string->attributes.find("index");
    if(it==field_string->attributes.end())
        return;

    const Schema::Field::ArrtibuteValue &index_option_value=it->second;
    if(std::get<bool>(index_option_value)){
        _indexes.insert({field_string->path,InvertIndex()});
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

void CollectionIndexes::search_fields(const std::unordered_map<string, std::vector<uint32_t> > &query_terms,
                                        const std::vector<string> &field_paths,Search::HitCollector &collector) const
{
    for(const std::string &path:field_paths){
        if(!contains(path)){
            throw std::runtime_error("field path \""+path+"\" is not found");
        }
        search_field(query_terms,path,collector);
    }
}

void CollectionIndexes::search_field(const std::unordered_map<string, std::vector<uint32_t>> &query_terms,
                                        const std::string &field_path,Search::HitCollector &collector) const
{
    /// \brief find the conjuction doc_ids of the query terms
    /// @example:
    /// query_term_A -> doc_id_1, doc_id_5, ...
    /// query_term_B -> doc_id_2, doc_id_4, doc_id_5...
    /// query_term_C -> doc_id_5, doc_id_7, doc_id_15, doc_id 29...
    /// conjuction result: doc_id_5 ...
    const InvertIndex *index=this->get_index(field_path);
    if(!index||query_terms.empty()){
        return;
    }

    std::vector<const PostingList*> term_entries;
    for(const auto &[query_term,offsets]:query_terms){
        auto *entry=index->get_postinglist(query_term);
        if(!entry) return;
        term_entries.push_back(entry);
    }
    std::sort(term_entries.begin(),term_entries.end(),[](const auto &e1,const auto &e2){
        return e1->num_docs()<e2->num_docs();
    });

    uint32_t leading_doc_num=term_entries[0]->num_docs();
    std::unique_ptr<uint32_t[]> leading_docs=term_entries[0]->get_all_doc_ids(); //shortest length, reduces the number of comparison
    for(uint32_t leading_cur=0;leading_cur<leading_doc_num;leading_cur++){
        uint32_t lead_doc=leading_docs[leading_cur];
        bool exists_in_all_entry=true;
        for(size_t i=1;i<term_entries.size();i++){
            //TODO 每次查找leading_doc 忽略掉前面不可能存在的部分
            if(!term_entries[i]->contain_doc(lead_doc)){ //couldn't find lead_doc in this entey
                exists_in_all_entry=false;
                break;
            }
        }
        if(exists_in_all_entry){
            Search::HitContext context;
            context.doc_seq_id=lead_doc;
            context.field_path=field_path;
            context._hit_postings=term_entries;
            context.field_doc_num=index->dict_size();
            collector.collect(context);
        }
    }
}

uint32_t CollectionIndexes::num_documents(const string &field_path, const string &term) const
{
    auto index_it=_indexes.find(field_path);
    if(index_it==_indexes.end()){
        return 0;
    }
    const InvertIndex &index=index_it->second;
    return index.num_docs(term);
}


} //namespace Index
} //namespace Skilo
