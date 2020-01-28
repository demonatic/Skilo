#include "Field.h"
#include <iostream>

namespace Skilo {
namespace Schema{

Field::Field(const std::string &name,const rapidjson::Value &schema)
{
    this->name=name;
    this->type=get_field_type(schema);
    get_arrtibutes(schema);

    if(type==FieldType::ARRAY){
        if(!schema.HasMember(item_keyword)){
            throw std::runtime_error("array type must have \""+std::string(item_keyword)+"\" keyword");
        }
        std::unique_ptr<Field> sub_field=create_field(item_keyword,schema[item_keyword]);
        this->sub_fields[sub_field->name]=std::move(sub_field);
    }
    if(type==FieldType::OBJECT){
        if(!schema.HasMember(field_keyword)){
            throw std::runtime_error("object type must have \""+std::string(field_keyword)+"\" keyword");
        }
        parse_sub_fields(schema[field_keyword]);
    }
}

Field::ArrtibuteValue Field::arrtibute(const std::string &attribute_name)
{
    auto it=attributes.find(attribute_name);
    return it!=attributes.end()?it->second:ArrtibuteValue{};
}

Field &Field::operator[](const std::string &sub_field_name)
{
    assert(sub_fields[sub_field_name]!=nullptr);
    return *sub_fields[sub_field_name];
}

void Field::get_arrtibutes(const rapidjson::Value &schema)
{
    if(schema.IsObject()){
        for(rapidjson::Value::ConstMemberIterator it=schema.MemberBegin();it!=schema.MemberEnd();++it){
            const std::string &arrtibute_name=it->name.GetString();
            auto keyword_it=Field::s_parsing_keywords.find(arrtibute_name);
            if(keyword_it!=Field::s_parsing_keywords.end()){
                continue;
            }
            ArrtibuteValue arrtibute_value;
            if(it->value.IsBool()){
                arrtibute_value=it->value.GetBool();
            }
            else if(it->value.IsInt()){
                arrtibute_value=it->value.GetInt();
            }
            else if(it->value.IsFloat()){
                arrtibute_value=it->value.GetFloat();
            }
            else if(it->value.IsString()){
                arrtibute_value=it->value.GetString();
            }
            attributes[arrtibute_name]=arrtibute_value;
        }
    }
}


void Field::parse_sub_fields(const rapidjson::Value &sub_schema)
{
    if(!sub_schema.IsObject()) return;
    for(rapidjson::Value::ConstMemberIterator it=sub_schema.MemberBegin();it!=sub_schema.MemberEnd();++it){
        std::unique_ptr<Field> sub_field=create_field(it->name.GetString(),it->value);
        if(sub_field){
            sub_fields[sub_field->name]=std::move(sub_field);
        }
    }
}

FieldType Field::get_field_type(const rapidjson::Value &schema)
{
    if(schema.HasMember("type")){
        const rapidjson::Value &type_val=schema["type"];
        if(!type_val.IsString()){
            throw std::runtime_error("type must be a string");
        }
        const std::string &type_str=type_val.GetString();
        if(type_str=="integer"){
            return FieldType::INTEGER;
        }
        else if(type_str=="float"){
            return FieldType::FLOAT;
        }
        else if(type_str=="string"){
            return FieldType::STRING;
        }
        else if(type_str=="array"){
            return FieldType::ARRAY;
        }
        else if(type_str=="boolean"){
            return FieldType::BOOLEAN;
        }
        else if(type_str=="object"){
            return FieldType::OBJECT;
        }
        else{
            throw std::runtime_error("unknown type keyword, type must be \"integer\"/\"float\"/\"string\"/\"array\"/\"boolean\"");
        }
    }
    else{
         throw std::runtime_error("neither \"$schema\" nor \"type\" keyword is defined");
    }
}

std::unique_ptr<Field> Field::create_field(const std::string &name,const rapidjson::Value &schema)
{
    FieldType type=Field::get_field_type(schema);
    switch(type){
        case FieldType::STRING:
            return std::make_unique<FieldString>(name,schema);

        case FieldType::INTEGER:
            return std::make_unique<FieldInteger>(name,schema);

        case FieldType::FLOAT:
            return std::make_unique<FieldFloat>(name,schema);

        case FieldType::BOOLEAN:
            return std::make_unique<FieldBoolean>(name,schema);

        case FieldType::ARRAY:
            return std::make_unique<FieldArray>(name,schema);

        case FieldType::OBJECT:
            return std::make_unique<FieldObject>(name,schema);
    }
    return nullptr;
}

void FieldString::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_string(this,document);
}

void FieldInteger::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_integer(this,document);
}

void FieldFloat::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_float(this,document);
}

void FieldBoolean::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_boolean(this,document);
}

void FieldObject::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_object(this,document);
    for(const auto &[field_name,field]:sub_fields){
        std::cout<<"@FieldObject::accept "<<field_name<<std::endl;
        rapidjson::Value::ConstMemberIterator it=document.FindMember(field_name.c_str());
        if(it==document.MemberEnd()){
            throw std::runtime_error("field \""+field_name+"\" not found when visit sub_field "
                                     +field_name+" from parent field \""+this->name+"\"");
        }
        field->accept(field_visitor,it->value);
    }
}

void FieldArray::accept(const FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    assert(document.IsArray());
    field_visitor.visit_field_array(this,document);
    for(const auto &elm:document.GetArray()){
        this->sub_fields[Field::item_keyword]->accept(field_visitor,elm);
    }
}

} //namespace Schema
} //namespace Skilo
