#include "InvertIndex.h"

namespace Skilo {
namespace Index{

InvertIndex::InvertIndex()
{

}

void InvertIndex::add_record(const IndexRecord &record)
{
    //TODO use offsets info if not empty
    for(const auto &[word,offsets]:record.word_offsets){
        string_view term=word;
        TermEntry *term_entry=_index.find(term.data(),term.length());
        if(!term_entry){
            term_entry=new TermEntry();
            _index.insert(term.data(),term.length(),term_entry);
        }
        term_entry->doc_freq++;
        cout<<"word="<<word<<" append_id="<<record.seq_id<<"freq="<<offsets.size()<<std::endl;
        term_entry->posting_list.add_doc(record.seq_id,offsets.size());
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
    if(std::get<bool>(index_option_value)){
        _indexes.insert({field_string->path,InvertIndex()});
    }
}

InvertIndex &CollectionIndexes::get_index(const string &field_path)
{
    return _indexes[field_path];
}

bool CollectionIndexes::contains(const string &field_path) const
{
    return _indexes.find(field_path)!=_indexes.end();
}


} //namespace Index
} //namespace Skilo
