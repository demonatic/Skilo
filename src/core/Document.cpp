#include "Document.h"
#include "../../3rd/include/rapidjson/stringbuffer.h"
#include "../../3rd/include/rapidjson/writer.h"
#include <iostream>
namespace Skilo {

DocumentBase::DocumentBase():_document(rapidjson::kObjectType)
{
    _document.AddMember("",0,_document.GetAllocator()); //TODO workaround for different allocator
}

DocumentBase::DocumentBase(const std::string_view json_str){
    if(_document.Parse(json_str.data(),json_str.length()).HasParseError()||!_document.IsObject()){
        throw Util::InvalidFormatException("Error when parse document from json, notice: root must be an json object");
    }
}

DocumentBase::DocumentBase(const SegmentBuf &json_str){
    detail::SegmentBufferStream stream(json_str);
    if(_document.ParseStream(stream).HasParseError()||!_document.IsObject()){
        throw Util::InvalidFormatException("Error when parse document from json, notice: root must be an json object");
    }
}

std::string DocumentBase::dump() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);
    return std::string(buffer.GetString(),buffer.GetSize());
}

rapidjson::Document &DocumentBase::get_raw()
{
    return _document;
}

const rapidjson::Document &DocumentBase::get_raw() const
{
    return _document;
}

Document::Document(rapidjson::Value doc):DocumentBase()
{
    _document.Swap(doc);
    this->extract_variables();
}

Document::Document(const std::string_view json_str):DocumentBase(json_str)
{
    this->extract_variables();
}

Document::Document(const SegmentBuf &json_str):DocumentBase(json_str)
{
    this->extract_variables();
}

void Document::extract_variables()
{
    rapidjson::Value::ConstMemberIterator seq_id_it=_document.FindMember("seq id");
    if(seq_id_it!=_document.MemberEnd()&&seq_id_it->value.IsUint()){
         _seq_id.emplace(seq_id_it->value.GetUint());
    }
}

uint32_t Document::get_doc_id() const
{
    rapidjson::Value::ConstMemberIterator doc_id_it=_document.FindMember("id");
    if(doc_id_it==_document.MemberEnd()||!doc_id_it->value.IsUint()){
         throw Util::InvalidFormatException("member \"id\" not found in document or \"id\" is not an unsigned integer");
    }
    return doc_id_it->value.GetUint();
}

void Document::add_seq_id(uint32_t seq_id)
{
    _seq_id.emplace(seq_id);
    rapidjson::Value value_id(rapidjson::kNumberType);
    value_id.SetUint(seq_id);
    _document.AddMember("seq id",value_id,_document.GetAllocator());
}

std::optional<uint32_t> Document::get_seq_id() const
{
    return _seq_id;
}

CollectionMeta::CollectionMeta(const std::string_view json_str):DocumentBase(json_str)
{
    this->extract_variables();
}

CollectionMeta::CollectionMeta(const SegmentBuf &json_str):DocumentBase(json_str)
{
    this->extract_variables();
}

uint32_t CollectionMeta::get_collection_id() const
{
    assert(_collection_id.has_value());
    return _collection_id.value();
}

const rapidjson::Value &CollectionMeta::get_schema() const
{
    rapidjson::Value::ConstMemberIterator schema_it=_document.FindMember("schema");
    if(schema_it==_document.MemberEnd()){
         throw Util::InvalidFormatException("missing \"schema\" in collection meta data");
    }
    return schema_it->value;
}

void CollectionMeta::extract_variables()
{
    rapidjson::Value::ConstMemberIterator collection_id_it=_document.FindMember("id");
    if(collection_id_it!=_document.MemberEnd()&&collection_id_it->value.IsUint()){
         _collection_id.emplace(collection_id_it->value.GetUint());
    }
    rapidjson::Value::ConstMemberIterator name_it=_document.FindMember("name");
    if(name_it==_document.MemberEnd()||!name_it->value.IsString()){
         throw Util::InvalidFormatException("missing \"name\" in collection meta data or \"name\" is not string");
    }
    this->_collection_name=name_it->value.GetString();
    rapidjson::Value::ConstMemberIterator schema_it=_document.FindMember("tokenizer");
    this->_tokenizer_name=(schema_it!=_document.MemberEnd()&&schema_it->value.IsString())?schema_it->value.GetString():"default";
}

const std::string& CollectionMeta::get_collection_name() const
{
    return _collection_name;
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
    _collection_id.emplace(collection_id);
}

const std::string& CollectionMeta::get_tokenizer() const
{
    return _tokenizer_name;
}

Query::Query(const std::string &collection_name,const std::string_view json_str):DocumentBase(json_str),_collection_name(collection_name)
{
    this->extract_variables();
}

Query::Query(const std::string &collection_name,const SegmentBuf &json_str):DocumentBase(json_str),_collection_name(collection_name)
{
    this->extract_variables();
}

const std::string &Query::get_collection_name() const
{
    return _collection_name;
}

const std::string &Query::get_search_str() const
{
    return _search_str;
}

const std::vector<std::string> &Query::get_query_fields() const
{
    return _query_fields;
}

void Query::extract_variables()
{
    rapidjson::Value::ConstMemberIterator query_it=_document.FindMember("query");
    if(query_it==_document.MemberEnd()||!query_it->value.IsString()){
        throw Util::InvalidFormatException("missing \"query\" in query json or \"query\" is not string");
    }
    _search_str=query_it->value.GetString();
    rapidjson::Value::ConstMemberIterator query_by_it=_document.FindMember("query by");
    if(query_by_it==_document.MemberEnd()||!query_by_it->value.IsArray()){
        throw Util::InvalidFormatException("missing \"query by\" in query json or \"query by\" is not string array");
    }
    for(const auto &query_field:query_by_it->value.GetArray()){
        if(!query_field.IsString()){
            throw Util::InvalidFormatException("\"query by\" must be string array");
        }
        _query_fields.emplace_back(query_field.GetString());
    }
}

SearchResult::SearchResult(uint32_t num_found)
{
    rapidjson::Value found(rapidjson::kNumberType);
    found.SetUint(num_found);
    _document.AddMember("found",found,_document.GetAllocator());
    rapidjson::Value hits(rapidjson::kArrayType);
    _document.AddMember("hits",hits,_document.GetAllocator());
}

void SearchResult::add_hit(Document &doc)
{
    rapidjson::Value d(std::move(doc.get_raw()),_document.GetAllocator());
    _document["hits"].GetArray().PushBack(d.Move(),_document.GetAllocator());
}

void SearchResult::add_took_ms(float ms)
{
    rapidjson::Value took_ms(rapidjson::kNumberType);
    took_ms.SetFloat(ms);
    _document.AddMember("took ms",took_ms,_document.GetAllocator());
}

DocumentBatch::DocumentBatch(const std::string &collection_name, const std::string_view json_str):
    DocumentBase(json_str),_collection_name(collection_name)
{
    this->extract_variables();
}

DocumentBatch::DocumentBatch(const std::string &collection_name, const SegmentBuf &json_str):
    DocumentBase(json_str),_collection_name(collection_name)
{
    this->extract_variables();
}

std::vector<Document> &DocumentBatch::get_docs()
{
    return _docs;
}

void DocumentBatch::extract_variables()
{
    rapidjson::Value::MemberIterator docs_it=_document.FindMember("docs");
    if(docs_it==_document.MemberEnd()||!docs_it->value.IsArray()){
        throw Util::InvalidFormatException("missing \"docs\" in document batch json or \"docs\" is not string array");
    }

    for(rapidjson::Value &doc_obj:docs_it->value.GetArray()){
        if(!doc_obj.IsObject()){
            throw Util::InvalidFormatException("\"each doc\" must be an object");
        }
        Document document(std::move(doc_obj));
        _docs.emplace_back(std::move(document));
    }
}


} //namespace Skilo
