#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>
#include <list>
#include <functional>
#include "../Document.h"

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
    "$fields": {
        "product name":{
            "type":"string"
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
            "$fields": {
                "length": {"type": "float"},
                "width": {"type": "float"},
                "height": {"type": "float"}
            }
        }
    }
}
*********************************************************/
enum class FieldType{
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    OBJECT,
    ARRAY,
};

enum class ValidateCode{
    OK,
    ERR_NOT_STRING,
    ERR_NOT_INTEGER,
    ERR_NOT_FLOAT,
    ERR_NOT_BOOLEAN,
    ERR_NOT_ARRAY,
    ERR_NOT_OBJECT
};

using Validator=std::function<ValidateCode(const rapidjson::Value &schema)>;

struct Field{
    Field(const std::string &name,const rapidjson::Value &schema);
    virtual ~Field()=default;
    virtual ValidateCode validate(const rapidjson::Value &schema);
    void parse_sub_fields(const rapidjson::Value &sub_schema);
    static FieldType get_field_type(const rapidjson::Value &schema);
    static std::unique_ptr<Field> create_field(const std::string &name,const rapidjson::Value &schema);

    std::string name;
    FieldType type;
    std::map<std::string,std::unique_ptr<Field>> sub_fields;
    std::vector<Validator> validators;
};

struct FieldString:Field{
    FieldString(const std::string &field_name,const rapidjson::Value &schema);
};

struct FieldInteger:Field{
    FieldInteger(const std::string &field_name,const rapidjson::Value &schema);
};

struct FieldFloat:Field{
    FieldFloat(const std::string &field_name,const rapidjson::Value &schema);
};

struct FieldBoolean:Field{
    FieldBoolean(const std::string &field_name,const rapidjson::Value &schema);
};

struct FieldObject:Field{
    FieldObject(const std::string &field_name,const rapidjson::Value &schema);

};

struct FieldArray:Field{
    FieldArray(const std::string &field_name,const rapidjson::Value &schema);

};

class CollectionSchema
{
public:


public:
    CollectionSchema(Document &schema);
    bool validate(const Document &document);

private:
    std::unique_ptr<Field> _fields;
};

} //namespace Schema
} //namespace Skilo

#endif // SCHEMA_H
