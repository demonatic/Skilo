#ifndef INDEXBUILDER_H
#define INDEXBUILDER_H

#include "../schema/Schema.h"
#include "Tokenizer.h"
#include "InvertIndex.h"

namespace Skilo {
namespace Index {

class IndexBuilder:public Schema::FieldVisitor
{
public:
    IndexBuilder(TokenizeStrategy *tokenizer);
    void index_in_memory(const Schema::CollectionSchema &schema,const Document &document);

    virtual void visit_field_string(const Schema::FieldString *field_string,const rapidjson::Value &document) const override;
    virtual void visit_field_integer(const Schema::FieldInteger *field_integer,const rapidjson::Value &document) const override;
    virtual void visit_field_float(const Schema::FieldFloat *field_float,const rapidjson::Value &document) const override;
    virtual void visit_field_boolean(const Schema::FieldBoolean *field_boolean,const rapidjson::Value &document) const override;
    virtual void visit_field_array(const Schema::FieldArray *field_array,const rapidjson::Value &document) const override;
    virtual void visit_field_object(const Schema::FieldObject *field_object,const rapidjson::Value &document) const override;

private:
    TokenizeStrategy *_tokenizer;
};

} //namespace Index
} //namespace Skilo


#endif // INDEXBUILDER_H
