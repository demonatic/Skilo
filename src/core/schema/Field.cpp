#include "Field.h"

namespace Skilo {
namespace Schema{

Field::Field(const std::string &name,const rapidjson::Value &schema)
{
    this->name=name;
    this->type=get_field_type(schema);
    get_arrtibutes(schema);

    if(type==FieldType::ARRAY){
        const char *item_keyword="$items";
        if(!schema.HasMember(item_keyword)){
            throw std::runtime_error("array type must have \""+std::string(item_keyword)+"\" keyword");
        }
        std::unique_ptr<Field> sub_field=create_field(item_keyword,schema[item_keyword]);
        this->sub_fields[sub_field->name]=std::move(sub_field);
    }
    if(type==FieldType::OBJECT){
        const char *field_keyword="$fields";
        if(!schema.HasMember(field_keyword)){
            throw std::runtime_error("object type must have \""+std::string(field_keyword)+"\" keyword");
        }
        parse_sub_fields(schema[field_keyword]);
    }
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
    std::unique_ptr<Field> field;
    switch(type){
        case FieldType::STRING:
            field=std::make_unique<FieldString>(name,schema);
        break;
        case FieldType::INTEGER:
            field=std::make_unique<FieldInteger>(name,schema);
        break;
        case FieldType::FLOAT:
            field=std::make_unique<FieldFloat>(name,schema);
        break;
        case FieldType::BOOLEAN:
            field=std::make_unique<FieldBoolean>(name,schema);
        break;
        case FieldType::ARRAY:
            field=std::make_unique<FieldArray>(name,schema);
        break;
        case FieldType::OBJECT:
            field=std::make_unique<FieldObject>(name,schema);
        break;
    }
    return field;
}

FieldString::FieldString(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsString()?ValidateCode::OK:ValidateCode::ERR_NOT_STRING;
    });
}

ValidateCode FieldString::validate(const rapidjson::Value &document)
{
    return ValidateCode::OK;
}

FieldInteger::FieldInteger(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsInt()?ValidateCode::OK:ValidateCode::ERR_NOT_INTEGER;
    });
}

ValidateCode FieldInteger::validate(const rapidjson::Value &document)
{

}

FieldFloat::FieldFloat(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsFloat()?ValidateCode::OK:ValidateCode::ERR_NOT_FLOAT;
    });
}

ValidateCode FieldFloat::validate(const rapidjson::Value &document)
{

}

FieldBoolean::FieldBoolean(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsBool()?ValidateCode::OK:ValidateCode::ERR_NOT_BOOLEAN;
    });
}

ValidateCode FieldBoolean::validate(const rapidjson::Value &document)
{

}

FieldObject::FieldObject(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsObject()?ValidateCode::OK:ValidateCode::ERR_NOT_OBJECT;
    });
}

ValidateCode FieldObject::validate(const rapidjson::Value &document)
{
    Schema::ValidateCode validate_code=ValidateCode::OK;
    for(FieldValidator validator:validators){
        validate_code=validator(document);
        if(validate_code!=ValidateCode::OK){
            return validate_code;
        }
    }
    for(const auto &[field_name,field]:sub_fields){
        rapidjson::Value::ConstMemberIterator it=document.FindMember(field_name.c_str());
        if(it==document.MemberEnd()){
            throw std::runtime_error("field \""+field_name+"\" not found");
        }
        validate_code=field->validate(it->value);
        if(validate_code!=ValidateCode::OK){
            break;
        }
    }
    return validate_code;
}

void FieldObject::accept(FieldVisitor &field_visitor, const rapidjson::Value &document)
{
    field_visitor.visit_field_object(this,document);
    for(const auto &[field_name,field]:sub_fields){
        rapidjson::Value::ConstMemberIterator it=document.FindMember(field_name.c_str());
        if(it==document.MemberEnd()){
            throw std::runtime_error("field \""+field_name+"\" not found when visit field "+field_name);
        }
        field->accept(field_visitor,it->value);
    }

}

FieldArray::FieldArray(const std::string &field_name,const rapidjson::Value &schema):Field(field_name,schema)
{
    validators.push_back([](const rapidjson::Value &schema){
        return schema.IsArray()?ValidateCode::OK:ValidateCode::ERR_NOT_ARRAY;
    });
}

ValidateCode FieldArray::validate(const rapidjson::Value &document)
{

}

} //namespace Schema
} //namespace Skilo
