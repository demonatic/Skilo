#include "IndexBuilder.h"

namespace Skilo {
namespace Index {

IndexBuilder::IndexBuilder(CollectionIndexes *indexes,TokenizeStrategy *tokenizer):_indexes(indexes),_tokenizer(tokenizer)
{

}

void IndexBuilder::index_in_memory(const Schema::CollectionSchema &schema, const Document &document)
{
    schema.get_root_field()->accept(*this,document.get_raw());
}

void IndexBuilder::visit_field_string(const Schema::FieldString *field_string, const rapidjson::Value &document)
{
    std::unordered_map<std::string, std::vector<uint32_t>> word_offsets=_tokenizer->tokenize(document.GetString());
    InvertIndex &index=_indexes->get_index(field_string->path);

}

void IndexBuilder::visit_field_integer(const Schema::FieldInteger *field_integer, const rapidjson::Value &document)
{

}

void IndexBuilder::visit_field_float(const Schema::FieldFloat *field_float, const rapidjson::Value &document)
{

}

void IndexBuilder::visit_field_boolean(const Schema::FieldBoolean *field_boolean, const rapidjson::Value &document)
{

}

void IndexBuilder::visit_field_array(const Schema::FieldArray *field_array, const rapidjson::Value &document)
{

}

} //namespace Index
} //namespace Skilo
