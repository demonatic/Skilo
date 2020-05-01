#ifndef INDEXBUILDER_H
#define INDEXBUILDER_H

#include "../schema/Schema.h"
#include "Tokenizer.h"
#include "Indexes.h"

namespace Skilo {
namespace Index {

class IndexWriter:public Schema::FieldVisitor
{
public:
    IndexWriter(CollectionIndexes &indexes,TokenizeStrategy *tokenizer);
    void index_in_memory(const Schema::CollectionSchema &schema,const Document &nodeument);

    virtual void visit_field_string(const Schema::FieldString *field_string,const rapidjson::Value &node) override;
    virtual void visit_field_integer(const Schema::FieldInteger *field_integer,const rapidjson::Value &node) override;
    virtual void visit_field_float(const Schema::FieldFloat *field_float,const rapidjson::Value &node) override;
    virtual void visit_field_boolean(const Schema::FieldBoolean *field_boolean,const rapidjson::Value &node) override;
    virtual void visit_field_array(const Schema::FieldArray *field_array,const rapidjson::Value &node) override;

private:
    uint32_t _seq_id;

    CollectionIndexes &_indexes;
    TokenizeStrategy *_tokenizer;
};

} //namespace Index
} //namespace Skilo


#endif // INDEXBUILDER_H
