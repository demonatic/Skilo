#ifndef FIELD_H
#define FIELD_H

#include <string>
#include <list>
#include <unordered_map>
#include <variant>
#include <functional>
#include <unordered_set>
#include "../Document.h"

namespace Skilo {
namespace Schema{

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

using FieldValidator=std::function<ValidateCode(const rapidjson::Value &schema)>;

struct FieldVisitor;
struct Field{
    using ArrtibuteValue=std::variant<std::string,bool,int,float>;
    inline static std::unordered_set<std::string> s_parsing_keywords{"type","$fields","$items"};
    //TODO object不能包含index attribute
    Field(const std::string &name,const rapidjson::Value &schema);
    virtual ~Field()=default;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document)=0;
    virtual ValidateCode validate(const rapidjson::Value &document)=0;

    void get_arrtibutes(const rapidjson::Value &schema);
    void parse_sub_fields(const rapidjson::Value &sub_schema);
    static FieldType get_field_type(const rapidjson::Value &schema);
    static std::unique_ptr<Field> create_field(const std::string &name,const rapidjson::Value &schema);


    std::string name;
    FieldType type;
    std::map<std::string,ArrtibuteValue> attributes;
    std::map<std::string,std::unique_ptr<Field>> sub_fields;
    std::vector<FieldValidator> validators;
};

struct FieldString:Field{
    FieldString(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldInteger:Field{
    FieldInteger(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldFloat:Field{
    FieldFloat(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldBoolean:Field{
    FieldBoolean(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldArray:Field{
    FieldArray(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldObject:Field{
    FieldObject(const std::string &field_name,const rapidjson::Value &schema);
    virtual ValidateCode validate(const rapidjson::Value &document) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldVisitor{
    virtual void visit_field_string(const FieldString *field_string,const rapidjson::Value &document)=0;
    virtual void visit_field_integer(const FieldInteger *field_integer,const rapidjson::Value &document)=0;
    virtual void visit_field_float(const FieldFloat *field_float,const rapidjson::Value &document)=0;
    virtual void visit_field_boolean(const FieldBoolean *field_boolean,const rapidjson::Value &document)=0;
    virtual void visit_field_array(const FieldArray *field_array,const rapidjson::Value &document)=0;
    virtual void visit_field_object(const FieldObject *field_object,const rapidjson::Value &document)=0;
};

} //namespace Schema
} //namespace Skilo
#endif // FIELD_H
