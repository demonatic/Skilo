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
    assert(seq_id.has_value());
    _seq_id=seq_id.value();
    schema.accept(*this,document.get_raw());
}

void IndexWriter::visit_field_string(const Schema::FieldString *field_string, const rapidjson::Value &node)
{
    InvertIndex *index=_indexes.get_invert_index(field_string->path);
    if(!index) return; //schema says we needn't index this field

    auto content=node.GetString();
    TokenSet token_set=_tokenizer->tokenize(content);

    uint32_t doc_len=node.GetStringLength();
    StrRecord record{_seq_id,doc_len,std::move(token_set.term_to_offsets())};
    index->index_str_record(record);

    if(field_string->attribute_val_true("suggest")){
        DefaultTokenizer segment_splitter;
        auto segments=segment_splitter.tokenize(content);
        for(auto &&[seg,offsets]:segments.term_to_offsets()){
            _indexes.get_suggestor()->update(seg);
        }
    }
}

void IndexWriter::visit_field_integer(const Schema::FieldInteger *field_integer, const rapidjson::Value &node)
{
    number_t integer_number=node.GetInt64();
    _indexes.index_numeric(field_integer->path,_seq_id,integer_number);
}

void IndexWriter::visit_field_float(const Schema::FieldFloat *field_float, const rapidjson::Value &node)
{
    number_t real_number=node.GetDouble();
    _indexes.index_numeric(field_float->path,_seq_id,real_number);
}

void IndexWriter::visit_field_boolean(const Schema::FieldBoolean *field_boolean, const rapidjson::Value &node)
{

}

void IndexWriter::visit_field_array(const Schema::FieldArray *field_array, const rapidjson::Value &node)
{

}

IndexEraser::IndexEraser(CollectionIndexes &indexes, TokenizeStrategy *tokenizer):_indexes(indexes),_tokenizer(tokenizer)
{

}

void IndexEraser::remove_from_memory_index(const Schema::CollectionSchema &schema, const Document &document)
{
    std::optional<uint32_t> seq_id=document.get_seq_id();
    assert(seq_id.has_value());
    _seq_id=seq_id.value();
    schema.accept(*this,document.get_raw());
}

void IndexEraser::visit_field_string(const Schema::FieldString *field_string, const rapidjson::Value &node)
{
    InvertIndex *index=_indexes.get_invert_index(field_string->path);
    if(!index) return; //schema says this field doesn't contain in index

    auto content=node.GetString();
    TokenSet token_set=_tokenizer->tokenize(content);

    uint32_t doc_len=strlen(content);
    StrRecord record{_seq_id,doc_len,std::move(token_set.term_to_offsets())};
    index->remove_str_record(record);

    if(field_string->attribute_val_true("suggest")){
        DefaultTokenizer segment_splitter;
        auto segments=segment_splitter.tokenize(content);
        for(auto &&[seg,offsets]:segments.term_to_offsets()){
            _indexes.get_suggestor()->de_update(seg);
        }
    }
}

void IndexEraser::visit_field_integer(const Schema::FieldInteger *field_integer, const rapidjson::Value &node)
{
    _indexes.remove_numeric(field_integer->path,_seq_id);
}

void IndexEraser::visit_field_float(const Schema::FieldFloat *field_float, const rapidjson::Value &node)
{
    _indexes.remove_numeric(field_float->path,_seq_id);
}

void IndexEraser::visit_field_boolean(const Schema::FieldBoolean *field_boolean, const rapidjson::Value &node)
{

}

void IndexEraser::visit_field_array(const Schema::FieldArray *field_array, const rapidjson::Value &node)
{

}

} //namespace Index
} //namespace Skilo
