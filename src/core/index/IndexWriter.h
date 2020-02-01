#ifndef INDEXBUILDER_H
#define INDEXBUILDER_H

#include "../schema/Schema.h"
#include "Tokenizer.h"
#include "InvertIndex.h"

namespace Skilo {
namespace Index {

class IndexWriter:public Schema::FieldVisitor
{
public:
    IndexWriter(CollectionIndexes &indexes,TokenizeStrategy *tokenizer);
    void index_in_memory(const Schema::CollectionSchema &schema,const Document &document);

    virtual void visit_field_string(const Schema::FieldString *field_string,const rapidjson::Value &document) override;
    virtual void visit_field_integer(const Schema::FieldInteger *field_integer,const rapidjson::Value &document) override;
    virtual void visit_field_float(const Schema::FieldFloat *field_float,const rapidjson::Value &document) override;
    virtual void visit_field_boolean(const Schema::FieldBoolean *field_boolean,const rapidjson::Value &document) override;
    virtual void visit_field_array(const Schema::FieldArray *field_array,const rapidjson::Value &document) override;

private:
    CollectionIndexes &_indexes;
    TokenizeStrategy *_tokenizer;

    uint32_t _seq_id;
};

} //namespace Index
} //namespace Skilo


#endif // INDEXBUILDER_H
