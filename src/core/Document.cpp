#include "Document.h"
#include <stdexcept>
#include "../../3rd/include/rapidjson/stringbuffer.h"
#include "../../3rd/include/rapidjson/writer.h"

namespace Skilo {

DocumentBase::DocumentBase(const std::string &json_str){
    if(_document.Parse(json_str.c_str()).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json");
    }
}

DocumentBase::DocumentBase(const SegmentBuf &json_str){
    detail::SegmentBufferStream stream(json_str);
    if(_document.ParseStream(stream).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json");
    }
}

std::string DocumentBase::dump() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);
    return std::string(buffer.GetString(),buffer.GetSize());
}

const rapidjson::Document &DocumentBase::get_raw() const
{
    return _document;
}

Document::Document(const std::string &collection_name,const std::string &json_str):
    DocumentBase(json_str),_collection_name(collection_name)
{

}

Document::Document(const std::string &collection_name,const SegmentBuf &json_str):
    DocumentBase(json_str),_collection_name(collection_name)
{

}

uint32_t Document::get_doc_id() const
{
    rapidjson::Value::ConstMemberIterator doc_id_it=_document.FindMember("id");
    if(doc_id_it==_document.MemberEnd()||!doc_id_it->value.IsUint()){
         throw std::runtime_error("member \"id\" not found in document or \"id\" is not an unsigned integer");
    }
    return doc_id_it->value.GetUint();
}

std::string Document::get_collection_name() const
{
    return _collection_name;
}

void Document::set_seq_id(uint32_t seq_id)
{
    _seq_id.emplace(seq_id);
}

std::optional<uint32_t> Document::get_seq_id() const
{
    assert(_seq_id.has_value());
    return _seq_id;
}

CollectionMeta::CollectionMeta(const std::string &json_str):DocumentBase(json_str)
{
    if(!_document.IsObject()){
        throw std::runtime_error("collection meta data must be a json object");
    }
}

CollectionMeta::CollectionMeta(const SegmentBuf &json_str):DocumentBase(json_str)
{
    if(!_document.IsObject()){
        throw std::runtime_error("collection meta data must be a json object");
    }
}

const rapidjson::Value &CollectionMeta::get_schema() const
{
    rapidjson::Value::ConstMemberIterator schema_it=_document.FindMember("schema");
    if(schema_it==_document.MemberEnd()){
         throw std::runtime_error("member \"schema\" not found in collection meta data");
    }
    return schema_it->value;
}

const char* CollectionMeta::get_collection_name() const
{
    rapidjson::Value::ConstMemberIterator name_it=_document.FindMember("name");
    if(name_it==_document.MemberEnd()||!name_it->value.IsString()){
         throw std::runtime_error("member \"name\" not found in collection meta data or \"name\" is not string");
    }
    return name_it->value.GetString();
}

void CollectionMeta::add_create_time(uint64_t created_time)
{
    rapidjson::Value create_time(rapidjson::kNumberType);
    create_time.SetUint64(created_time);
    _document.AddMember("created_at",created_time,_document.GetAllocator());
}

void CollectionMeta::add_collection_id(uint32_t collection_id)
{
    rapidjson::Value collec_id(rapidjson::kNumberType);
    collec_id.SetUint(collection_id);
    _document.AddMember("id",collec_id,_document.GetAllocator());
}


} //namespace Skilo
