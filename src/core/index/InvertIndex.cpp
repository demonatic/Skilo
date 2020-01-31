#include "InvertIndex.h"

namespace Skilo {
namespace Index{

InvertIndex::InvertIndex()
{

}

void InvertIndex::add_record(const IndexRecord &record)
{
    for(size_t i=0;i<record.tokens.size();i++){
        string_view term=record.tokens[i];
        TermEntry *term_entry=_index.find(term.data(),term.length());
        if(!term_entry){
            term_entry=new TermEntry();
            _index.insert(term.data(),term.length(),term_entry);
        }
        term_entry->doc_freq++;
        term_entry->posting_list.add_doc(record.seq_id);
    }
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
    if(std::get<std::string>(index_option_value)=="true"){
        _indexes.insert({field_string->path,InvertIndex()});
    }
}


} //namespace Index
} //namespace Skilo
