#ifndef SCHEMA_H
#define SCHEMA_H

#include "Field.h"

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
    virtual void visit_field_string(const FieldString &field_string)=0;
    virtual void visit_field_integer(const FieldInteger &field_integer)=0;
    virtual void visit_field_float(const FieldFloat &field_float)=0;
    virtual void visit_field_boolean(const FieldBoolean &field_boolean)=0;
    virtual void visit_field_array(const FieldArray &field_array)=0;
    virtual void visit_field_object(const FieldString &field_object)=0;
};

class CollectionSchema
{
public:
    CollectionSchema(const Document &schema);
    bool validate(const Document &document) const;

private:
    std::unique_ptr<Field> _fields;
};

} //namespace Schema
} //namespace Skilo

#endif // SCHEMA_H
