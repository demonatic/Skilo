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
    using AttributeValue=std::variant<std::string,bool,int,float>;
    std::string name;
    std::string path;
    FieldType type;
    std::unordered_map<std::string,AttributeValue> attributes;
    std::unordered_map<std::string,std::unique_ptr<Field>> sub_fields;

    Field(const std::string &name,const std::string &path,const rapidjson::Value &schema);
    virtual ~Field()=default;
    static std::unique_ptr<Field> create_field(const std::string &name,const std::string &path,const rapidjson::Value &schema);

    virtual void accept(FieldVisitor &field_visitor)=0;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document)=0;

    AttributeValue attribute(const std::string &attribute_name) const;
    Field &operator[](const std::string &sub_field_name);

    bool attribute_val_true(const std::string &attribute_name) const;

protected:
    static FieldType get_field_type(const rapidjson::Value &schema);

    void parse_attributes(const rapidjson::Value &schema);
    void parse_sub_fields(const rapidjson::Value &sub_schema,const std::string &path);

    inline static const char *item_keyword="$items";
    inline static const char *field_keyword="$fields";
    inline static std::unordered_set<std::string> s_parsing_keywords{"type",item_keyword,field_keyword};
};

struct FieldString:Field{
    FieldString(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldInteger:Field{
    FieldInteger(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldFloat:Field{
    FieldFloat(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldBoolean:Field{
    FieldBoolean(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldArray:Field{
    FieldArray(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldObject:Field{
    FieldObject(const std::string &field_name,const std::string &field_path,const rapidjson::Value &schema):Field(field_name,field_path,schema){}
    virtual void accept(FieldVisitor &field_visitor) override;
    virtual void accept(FieldVisitor &field_visitor,const rapidjson::Value &document) override;
};

struct FieldVisitor{
    virtual ~FieldVisitor()=default;
    virtual void visit_field_string(const FieldString *field_string){}
    virtual void visit_field_integer(const FieldInteger *field_integer){}
    virtual void visit_field_float(const FieldFloat *field_float){}
    virtual void visit_field_boolean(const FieldBoolean *field_boolean){}
    virtual void visit_field_array(const FieldArray *field_array){}
    virtual void visit_field_object(const FieldObject *field_object){}

    virtual void visit_field_string(const FieldString *field_string,const rapidjson::Value &document){}
    virtual void visit_field_integer(const FieldInteger *field_integer,const rapidjson::Value &document){}
    virtual void visit_field_float(const FieldFloat *field_float,const rapidjson::Value &document){}
    virtual void visit_field_boolean(const FieldBoolean *field_boolean,const rapidjson::Value &document){}
    virtual void visit_field_array(const FieldArray *field_array,const rapidjson::Value &document){}
    virtual void visit_field_object(const FieldObject *field_object,const rapidjson::Value &document){}
};

} //namespace Schema
} //namespace Skilo
#endif // FIELD_H
