#include "IndexWriter.h"
#include "g3log/g3log.hpp"

namespace Skilo {
namespace Index {

IndexWriter::IndexWriter(CollectionIndexes &indexes,TokenizeStrategy *tokenizer):_indexes(indexes),_tokenizer(tokenizer)
{

}

void IndexWriter::index_in_memory(const Schema::CollectionSchema &schema, const Document &document)
{
    std::optional<uint32_t> seq_id=document.get_seq_id();
    _seq_id=seq_id.value();
    schema.accept(*this,document.get_raw());
}

void IndexWriter::visit_field_string(const Schema::FieldString *field_string, const rapidjson::Value &document)
{
    InvertIndex *index=_indexes.get_index(field_string->path);
    if(!index) return;

    cout<<"string="<<document.GetString()<<endl;
    cout<<"path="<<field_string->path<<std::endl;
    std::unordered_map<std::string, std::vector<uint32_t>> word_offsets;
    word_offsets=_tokenizer->tokenize(document.GetString());

    IndexRecord record{_seq_id,std::move(word_offsets)};
    index->add_record(record);
}

void IndexWriter::visit_field_integer(const Schema::FieldInteger *field_integer, const rapidjson::Value &document)
{

}

void IndexWriter::visit_field_float(const Schema::FieldFloat *field_float, const rapidjson::Value &document)
{

}

void IndexWriter::visit_field_boolean(const Schema::FieldBoolean *field_boolean, const rapidjson::Value &document)
{

}

void IndexWriter::visit_field_array(const Schema::FieldArray *field_array, const rapidjson::Value &document)
{

}

} //namespace Index
} //namespace Skilo
