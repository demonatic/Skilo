#include "Document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <iostream>
namespace Skilo {

DocumentBase::DocumentBase():_document(rapidjson::kObjectType)
{

}

DocumentBase::DocumentBase(const std::string_view json_str){
    if(_document.Parse(json_str.data(),json_str.length()).HasParseError()||!_document.IsObject()){
        throw InvalidFormatException("Error when parse document from json, notice: root must be an json object");
    }
}

DocumentBase::DocumentBase(const SegmentBuf &json_str){
    detail::SegmentBufferStream stream(json_str);
    if(_document.ParseStream(stream).HasParseError()||!_document.IsObject()){
        throw InvalidFormatException("Error when parse document from json, notice: root must be an json object");
    }
}

bool DocumentBase::contain_key(const std::string &key) const
{
    rapidjson::Value::ConstMemberIterator docs_it=_document.FindMember(key.c_str());
    return docs_it!=_document.MemberEnd();
}

std::string DocumentBase::dump() const
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);
    return std::string(buffer.GetString(),buffer.GetSize());
}

std::optional<uint32_t> DocumentBase::extract_opt_int(const rapidjson::Value &v, const std::string &field_name) const
{
    std::optional<uint32_t> opt;
    auto field_it=v.FindMember(field_name.c_str());
    if(field_it!=v.MemberEnd()){
        if(field_it->value.IsUint()){
            opt=std::make_optional<uint32_t>(field_it->value.GetUint());
        }
        else{
            throw InvalidFormatException("\""+field_name+"\" must be an unsigned integer");
        }
    }
    return opt;
}

rapidjson::Document &DocumentBase::get_raw()
{
    return _document;
}

const rapidjson::Document &DocumentBase::get_raw() const
{
    return _document;
}


Document::Document(DocumentBase &base)
{
    base.get_raw().Swap(this->_document);
    this->extract_variables();
}

Document::Document(rapidjson::Value doc, decltype(_document.GetAllocator()) &allocator)
{
    _document=rapidjson::Document(rapidjson::kObjectType,&allocator);
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
         throw InvalidFormatException("member \"id\" not found in document or \"id\" is not an unsigned integer");
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

rapidjson::Value &Document::get_value(const std::string &field_path)
{
    std::string_view path=field_path;
    rapidjson::Value *v=&this->_document;
    while(path.length()){
        if(!v->IsObject()&&!v->IsArray()){
            throw NotFoundException("invalid field path \""+field_path+"\"");
        }
        size_t delimiter_index=path.find_first_of('.');
        std::string_view field_name=delimiter_index==std::string::npos?path:path.substr(0,delimiter_index);

        if(v->IsObject()){
            auto it=v->FindMember(std::string(field_name).c_str());
            if(it==v->MemberEnd()){
                throw NotFoundException("invalid field path \""+field_path+"\"");
            }
            v=&it->value;
        }
        else if(v->IsArray()){
            try {
                auto index=std::stoi(std::string(field_name));
                v=&v->GetArray()[index];
            }  catch (std::invalid_argument &) {
                 throw NotFoundException("invalid field path \""+field_path+"\"");
            }
        }
        else{
            throw NotFoundException("invalid field path \""+field_path+"\"");
        }

        path.remove_prefix(delimiter_index==std::string::npos?path.length():delimiter_index+1);
    }
    return *v;
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
         throw InvalidFormatException("missing \"schema\" in collection meta data");
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
         throw InvalidFormatException("missing \"name\" in collection meta data or \"name\" is not string");
    }
    this->_collection_name=name_it->value.GetString();
    rapidjson::Value::ConstMemberIterator tokenizer_it=_document.FindMember("tokenizer");
    this->_tokenizer_name=(tokenizer_it!=_document.MemberEnd()&&tokenizer_it->value.IsString())?
                tokenizer_it->value.GetString():"default";

    rapidjson::Value::ConstMemberIterator auto_sugg_it=_document.FindMember("auto_suggestion");
    _enable_auto_suggestion=!(auto_sugg_it==_document.MemberEnd());

    if(_enable_auto_suggestion){
        if(!auto_sugg_it->value.IsObject()){
            throw InvalidFormatException("\"auto_suggestion\" must be an object");
        }
        const rapidjson::Value &auto_sugg=auto_sugg_it->value;
        _min_gram=extract_opt_int(auto_sugg,"min_gram");
        _max_gram=extract_opt_int(auto_sugg,"max_gram");
        _suggest_entry_num=extract_opt_int(auto_sugg,"entry_num");
    }
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

uint32_t CollectionMeta::get_min_gram(const uint32_t default_min_gram) const
{
    return _min_gram.value_or(default_min_gram);
}

uint32_t CollectionMeta::get_max_gram(const uint32_t default_max_gram) const
{
    return _max_gram.value_or(default_max_gram);
}

uint32_t CollectionMeta::get_suggest_entry_num(const uint32_t default_suggest_entry_num) const
{
    return _suggest_entry_num.value_or(default_suggest_entry_num);
}

const std::string& CollectionMeta::get_tokenizer() const
{
    return _tokenizer_name;
}

bool CollectionMeta::enable_auto_suggestion() const
{
    return _enable_auto_suggestion;
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

const std::vector<std::pair<std::string,Query::SortInAscend>> &Query::get_sort_fields() const
{
    return _sort_fields;
}

void Query::extract_variables()
{
    rapidjson::Value::ConstMemberIterator query_it=_document.FindMember("query");
    if(query_it==_document.MemberEnd()||!query_it->value.IsString()){
        throw InvalidFormatException("missing \"query\" in query json or \"query\" is not string");
    }
    _search_str=query_it->value.GetString();
    rapidjson::Value::ConstMemberIterator query_by_it=_document.FindMember("query by");
    if(query_by_it==_document.MemberEnd()||!query_by_it->value.IsArray()){
        throw InvalidFormatException("missing \"query by\" in query json or \"query by\" is not string array");
    }
    for(const auto &query_field:query_by_it->value.GetArray()){
        if(!query_field.IsString()){
            throw InvalidFormatException("\"query by\" must be string array");
        }
        _query_fields.emplace_back(query_field.GetString());
    }

    rapidjson::Value::ConstMemberIterator sort_by_it=_document.FindMember("sort by");
    if(sort_by_it==_document.MemberEnd()){
        return;
    }
    if(!sort_by_it->value.IsArray()){
        throw InvalidFormatException("\"sort by\" is not string array");
    }
    for(const auto &sort_field:sort_by_it->value.GetArray()){
        if(!sort_field.IsString()){
            throw InvalidFormatException("\"sort by\" must be string array");
        }
        std::string_view sort_str=sort_field.GetString();
        size_t delimiter_index=sort_str.find(':');
        if(delimiter_index==std::string_view::npos||delimiter_index==sort_str.length()-1){
            throw InvalidFormatException("missing order in \"sort by\"");
        }
        std::string_view sort_order=sort_str.substr(delimiter_index+1);

        SortInAscend is_ascend;
        if(sort_order=="desc"){
            is_ascend=false;
        }
        else if(sort_order=="asc"){
            is_ascend=true;
        }
        else{
            throw InvalidFormatException(R"("sort by" order should be either "desc" or "asc")");
        }
        _sort_fields.emplace_back(sort_str.substr(0,delimiter_index),is_ascend);
    }
}

SearchResult::SearchResult(uint32_t num_found)
{
    rapidjson::Value found(rapidjson::kNumberType);
    found.SetUint(num_found);
    _document.AddMember("found",found,_document.GetAllocator());
    rapidjson::Value hits(rapidjson::kArrayType);
    _document.AddMember("hits",hits,_document.GetAllocator());
    rapidjson::Value scores(rapidjson::kArrayType);
    _document.AddMember("scores",scores,_document.GetAllocator());
}

void SearchResult::add_hit(Document &doc,double score)
{
    rapidjson::Value d(std::move(doc.get_raw()),_document.GetAllocator());
    _document["hits"].GetArray().PushBack(d.Move(),_document.GetAllocator());
    _document["scores"].GetArray().PushBack(score,_document.GetAllocator());
}

void SearchResult::add_took_ms(float ms)
{
    rapidjson::Value took_ms(rapidjson::kNumberType);
    took_ms.SetFloat(ms);
    _document.AddMember("took ms",took_ms,_document.GetAllocator());
}

DocumentBatch::DocumentBatch(DocumentBase &base)
{
    base.get_raw().Swap(this->_document);
    this->extract_variables();
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
        throw InvalidFormatException("missing \"docs\" in document batch json or \"docs\" is not string array");
    }

    for(rapidjson::Value &doc_obj:docs_it->value.GetArray()){
        if(!doc_obj.IsObject()){
            throw InvalidFormatException("\"each doc\" must be an object");
        }
        Document document(std::move(doc_obj),this->_document.GetAllocator());
        _docs.emplace_back(std::move(document));
    }
}

AutoSuggestion::AutoSuggestion()
{
    rapidjson::Value suggestions(rapidjson::kArrayType);
    _document.AddMember("suggestions",suggestions,_document.GetAllocator());
}

void AutoSuggestion::add_suggestion(std::string_view suggestion)
{
    rapidjson::GenericStringRef<char> sug(suggestion.data(),suggestion.length());
    _document["suggestions"].GetArray().PushBack(sug,_document.GetAllocator());
}


} //namespace Skilo
