#include "Schema.h"
#include <exception>
#include <iostream>

namespace Skilo{
namespace Schema{

CollectionSchema::CollectionSchema(const CollectionMeta &meta_data)
{
    this->_fields=Field::create_field("$schema",meta_data.get_schema());
}

CollectionSchema::CollectionSchema(CollectionSchema &&collection_schema)
{
    _fields=std::move(collection_schema._fields);
    _validator=collection_schema._validator;
}

std::optional<std::string> CollectionSchema::validate(const Document &document) const
{
    try {
        _fields->accept(_validator,document.get_raw());
    }  catch (const std::runtime_error &err) {
        return std::make_optional<std::string>(err.what());
    }
    return std::nullopt;
}

Field *CollectionSchema::get_root_field() const
{
    return this->_fields.get();
}

void SchemaValidator::visit_field_string(const FieldString *field_string, const rapidjson::Value &document) const
{
    std::cout<<"visit "<<field_string->name<<std::endl;
    if(!document.IsString()){
        throw std::runtime_error("field \""+field_string->name+"\" should be a string");
    }
}

void SchemaValidator::visit_field_integer(const FieldInteger *field_integer, const rapidjson::Value &document) const
{
    std::cout<<"visit "<<field_integer->name<<std::endl;
    if(!document.IsInt()){
        throw std::runtime_error("field \""+field_integer->name+"\" should be an integer");
    }
}

void SchemaValidator::visit_field_float(const FieldFloat *field_float, const rapidjson::Value &document) const
{
    if(!document.IsFloat()){
        throw std::runtime_error("field \""+field_float->name+"\" should be a float");
    }
}

void SchemaValidator::visit_field_boolean(const FieldBoolean *field_boolean, const rapidjson::Value &document) const
{
    std::cout<<"visit "<<field_boolean->name<<std::endl;
    if(!document.IsBool()){
         throw std::runtime_error("field \""+field_boolean->name+"\" should be a boolean");
    }
}

void SchemaValidator::visit_field_array(const FieldArray *field_array, const rapidjson::Value &document) const
{
    std::cout<<"visit "<<field_array->name<<std::endl;
    if(!document.IsArray()){
         throw std::runtime_error("field \""+field_array->name+"\" should be an array");
    }
}

void SchemaValidator::visit_field_object(const FieldObject *field_object, const rapidjson::Value &document) const
{
    std::cout<<"visit "<<field_object->name<<std::endl;
    if(!document.IsObject()){
         throw std::runtime_error("field \""+field_object->name+"\" should be an object");
    }
}



} //namespace Schema
}//namespace Schema
