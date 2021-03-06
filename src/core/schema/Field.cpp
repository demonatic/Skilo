#include "Field.h"
#include <iostream>

namespace Skilo {
namespace Schema{

Field::Field(const std::string &name,const std::string &path,const rapidjson::Value &schema)
{
    this->name=name;
    this->type=get_field_type(schema);
    this->path=path.empty()?name:path+"."+name;
    parse_attributes(schema);

    if(type==FieldType::ARRAY){
        if(!schema.HasMember(item_keyword)){
            throw InvalidFormatException("array type must have \""+std::string(item_keyword)+"\" keyword");
        }
        std::unique_ptr<Field> sub_field=create_field(item_keyword,this->path,schema[item_keyword]);
        this->sub_fields[sub_field->name]=std::move(sub_field);
    }
    if(type==FieldType::OBJECT){
        if(!schema.HasMember(field_keyword)){
            throw InvalidFormatException("object type must have \""+std::string(field_keyword)+"\" keyword");
        }
        parse_sub_fields(schema[field_keyword],this->path);
    }
}

Field::AttributeValue Field::attribute(const std::string &attribute_name) const
{
    auto it=attributes.find(attribute_name);
    return it!=attributes.end()?it->second:AttributeValue{};
}

Field &Field::operator[](const std::string &sub_field_name)
{
    assert(sub_fields[sub_field_name]!=nullptr);
    return *sub_fields[sub_field_name];
}

bool Field::attribute_val_true(const std::string &attribute_name) const
{
    Schema::Field::AttributeValue attr=this->attribute(attribute_name);
    bool* ptr=std::get_if<bool>(&attr);
    return ptr&&(*ptr);
}

void Field::parse_attributes(const rapidjson::Value &schema)
{
    if(schema.IsObject()){
        for(rapidjson::Value::ConstMemberIterator it=schema.MemberBegin();it!=schema.MemberEnd();++it){
            const std::string &attribute_name=it->name.GetString();
            auto keyword_it=Field::s_parsing_keywords.find(attribute_name);
            if(keyword_it!=Field::s_parsing_keywords.end()){
                continue;
            }
            AttributeValue attribute_value;
            if(it->value.IsBool()){
                attribute_value=it->value.GetBool();
            }
            else if(it->value.IsInt()){
                attribute_value=it->value.GetInt();
            }
            else if(it->value.IsFloat()){
                attribute_value=it->value.GetFloat();
            }
            else if(it->value.IsString()){
                attribute_value=it->value.GetString();
            }
            attributes[attribute_name]=attribute_value;
        }
    }
}

void Field::parse_sub_fields(const rapidjson::Value &sub_schema,const std::string &path)
{
    if(!sub_schema.IsObject()) return;
    for(rapidjson::Value::ConstMemberIterator it=sub_schema.MemberBegin();it!=sub_schema.MemberEnd();++it){
        std::unique_ptr<Field> sub_field=create_field(it->name.GetString(),path,it->value);
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
            throw InvalidFormatException("type must be a string");
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
            throw InvalidFormatException("unknown type keyword, type must be \"integer\"/\"float\"/\"string\"/\"array\"/\"boolean\"");
        }
    }
    else{
         throw InvalidFormatException("neither \"$schema\" nor \"type\" keyword is defined");
    }
}

std::unique_ptr<Field> Field::create_field(const std::string &name,const std::string &path,const rapidjson::Value &schema)
{
    FieldType type=Field::get_field_type(schema);
    switch(type){
        case FieldType::STRING:
            return std::make_unique<FieldString>(name,path,schema);

        case FieldType::INTEGER:
            return std::make_unique<FieldInteger>(name,path,schema);

        case FieldType::FLOAT:
            return std::make_unique<FieldFloat>(name,path,schema);

        case FieldType::BOOLEAN:
            return std::make_unique<FieldBoolean>(name,path,schema);

        case FieldType::ARRAY:
            return std::make_unique<FieldArray>(name,path,schema);

        case FieldType::OBJECT:
            return std::make_unique<FieldObject>(name,path,schema);
    }
    return nullptr;
}


void FieldObject::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_object(this);
    for(const auto &[field_name,field]:sub_fields){
        field->accept(field_visitor);
    }
}

void FieldObject::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_object(this,document);
    for(const auto &[field_name,field]:sub_fields){
        rapidjson::Value::ConstMemberIterator it=document.FindMember(field_name.c_str());
        if(it==document.MemberEnd()){
            throw InvalidFormatException("field \""+field_name+"\" not found when visit sub_field "
                                     +field_name+" from path:\""+this->path+"\"");
        }
        field->accept(field_visitor,it->value);
    }
}

void FieldArray::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_array(this);
    this->sub_fields[item_keyword]->accept(field_visitor);
}

void FieldArray::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    assert(document.IsArray());
    field_visitor.visit_field_array(this,document);
    for(const auto &elm:document.GetArray()){
        this->sub_fields[Field::item_keyword]->accept(field_visitor,elm);
    }
}

void FieldString::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_string(this);
}

void FieldString::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_string(this,document);
}

void FieldInteger::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_integer(this);
}

void FieldInteger::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_integer(this,document);
}

void FieldFloat::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_float(this);
}

void FieldFloat::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_float(this,document);
}

void FieldBoolean::accept(FieldVisitor &field_visitor)
{
    field_visitor.visit_field_boolean(this);
}

void FieldBoolean::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_boolean(this,document);
}

} //namespace Schema
} //namespace Skilo
