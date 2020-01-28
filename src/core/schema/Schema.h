#ifndef SCHEMA_H
#define SCHEMA_H

#include "Field.h"
#include <optional>

namespace Skilo {
namespace Schema{

/********************************************************
 * The Document string example:
{
    "product name":"Car Model",
    "product id":"1001",
    "price":"6.5",
    "composition":["Engine","Wheel","Body"],
    "dimensions":{
        "length":"12.0",
        "width":"4.5",
        "height":"3.0"
    }
}
 ********************************************************
 *The corresponding Schema is:
{
    "type":"object",
    "$fields": {
        "product name":{
            "type":"string"
            "index":true
        },
        "product id":{
            "type":"integer"
        },
        "price":{
            "type":"float"
        },
        "composition": {
            "type": "array",
            "$items": {
                "type": "string"
            }
        },
        "dimensions": {
            "type":"object",
            "$fields": {
                "length": {"type": "float"},
                "width": {"type": "float"},
                "height": {"type": "float"}
            }
        }
    }
}
*********************************************************/

struct SchemaValidator:FieldVisitor{
    virtual void visit_field_string(const FieldString *field_string,const rapidjson::Value &document) const override;
    virtual void visit_field_integer(const FieldInteger *field_integer,const rapidjson::Value &document) const override;
    virtual void visit_field_float(const FieldFloat *field_float,const rapidjson::Value &document) const override;
    virtual void visit_field_boolean(const FieldBoolean *field_boolean,const rapidjson::Value &document) const override;
    virtual void visit_field_array(const FieldArray *field_array,const rapidjson::Value &document) const override;
    virtual void visit_field_object(const FieldObject *field_object,const rapidjson::Value &document) const override;
};

class CollectionSchema
{
public:
    CollectionSchema(const Document &schema);

    /// @brief check whether the document adhere to the corresponding schema
    /// @return error string if any error occurs
    std::optional<std::string> validate(const Document &document) const;

private:
    std::unique_ptr<Field> _fields;
    SchemaValidator _validator;
};

} //namespace Schema
} //namespace Skilo

#endif // SCHEMA_H
