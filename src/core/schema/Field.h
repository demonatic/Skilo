#ifndef FIELD_H
#define FIELD_H

#include <string>
#include <list>
#include <unordered_map>
#include <variant>
#include <functional>
#include <unordered_set>
#include <memory>
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

struct FieldVisitor;
struct Field{
    using ArrtibuteValue=std::variant<std::string,bool,int,float>;
    std::string name;
    FieldType type;
    std::unordered_map<std::string,ArrtibuteValue> attributes;
    std::unordered_map<std::string,std::unique_ptr<Field>> sub_fields;

    //TODO object不能包含index attribute
    Field(const std::string &name,const rapidjson::Value &schema);
    virtual ~Field()=default;
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document)=0;

    ArrtibuteValue arrtibute(const std::string &arrtibute_name);
    Field &operator[](const std::string &sub_field_name);

    static FieldType get_field_type(const rapidjson::Value &schema);
    static std::unique_ptr<Field> create_field(const std::string &name,const rapidjson::Value &schema);

    void get_arrtibutes(const rapidjson::Value &schema);
    void parse_sub_fields(const rapidjson::Value &sub_schema);

    inline static const char *item_keyword="$items";
    inline static const char *field_keyword="$fields";
    inline static std::unordered_set<std::string> s_parsing_keywords{"type",item_keyword,field_keyword};
};

struct FieldString:Field{
    FieldString(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldInteger:Field{
    FieldInteger(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldFloat:Field{
    FieldFloat(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldBoolean:Field{
    FieldBoolean(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldArray:Field{
    FieldArray(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldObject:Field{
    FieldObject(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema){}
    virtual void accept(const FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldVisitor{
    virtual ~FieldVisitor()=default;
    virtual void visit_field_string(const FieldString *field_string,const rapidjson::Value &document) const=0;
    virtual void visit_field_integer(const FieldInteger *field_integer,const rapidjson::Value &document) const=0;
    virtual void visit_field_float(const FieldFloat *field_float,const rapidjson::Value &document) const=0;
    virtual void visit_field_boolean(const FieldBoolean *field_boolean,const rapidjson::Value &document) const=0;
    virtual void visit_field_array(const FieldArray *field_array,const rapidjson::Value &document) const=0;
    virtual void visit_field_object(const FieldObject *field_object,const rapidjson::Value &document) const=0;
};

} //namespace Schema
} //namespace Skilo
#endif // FIELD_H
