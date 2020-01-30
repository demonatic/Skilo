#include "IndexBuilder.h"

namespace Skilo {
namespace Index {

IndexBuilder::IndexBuilder(TokenizeStrategy *tokenizer):_tokenizer(tokenizer)
{

}

void IndexBuilder::index_in_memory(const Schema::CollectionSchema &schema, const Document &document)
{
    schema.get_root_field()->accept(*this,document.get_raw());
}

void IndexBuilder::visit_field_string(const Schema::FieldString *field_string, const rapidjson::Value &document) const
{

}

} //namespace Index
} //namespace Skilo
