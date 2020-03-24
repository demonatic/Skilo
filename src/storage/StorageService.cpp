#include "StorageService.h"
#include "KeyConverter.hpp"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "g3log/g3log.hpp"
#include <iostream>
namespace Skilo {
namespace Storage{

StorageService::StorageService(const std::string &db_path):_storage_engine(db_path)
{

}

bool StorageService::contain_collection(const std::string &collection_name) const
{
    const std::string collection_meta_key=KeyConverter::collection_meta_key(collection_name);
    return _storage_engine.contains(collection_meta_key);
}

bool StorageService::contain_document(const uint32_t collection_id, uint32_t doc_id) const
{
    const std::string &doc_key=KeyConverter::doc_id_key(collection_id,doc_id);
    return _storage_engine.contains(doc_key);
}

uint32_t StorageService::get_next_collection_id() const
{
    std::string next_collection_val="0";
    StorageEngine::Status status=_storage_engine.get(KeyConverter::next_collection_id_key(),next_collection_val);
    if(status==StorageEngine::ERROR){
        std::string err="Error when getting collection manager's next collection id from disk";
        LOG(FATAL)<<err;
        throw InternalServerException(err);
    }
    uint32_t next_collection_id=std::stoul(next_collection_val);
    return next_collection_id;
}

uint32_t StorageService::get_collection_next_seq_id(uint32_t collection_id) const
{
    std::string next_seq_val;
    StorageEngine::Status next_seq_status=_storage_engine.get(KeyConverter::collection_next_seq_key(collection_id),next_seq_val);
    if(next_seq_status!=StorageEngine::FOUND){
        std::string err="Can not get collection next sequence id of collection id=\""+std::to_string(collection_id)+"\"";
        LOG(WARNING)<<err;
        throw InternalServerException(err);
    }
    uint32_t next_seq_id=std::stoul(next_seq_val);
    return next_seq_id;
}

Document StorageService::get_document(const uint32_t collection_id,const uint32_t seq_id) const
{
    const std::string &seq_key=KeyConverter::doc_seq_key(collection_id,seq_id);
    std::string doc_json_str;
    StorageEngine::Status status=_storage_engine.get(seq_key,doc_json_str);
    if(status!=StorageEngine::FOUND){
        std::string err="Can not fetch document of sequence id \""+std::to_string(seq_id)+"\"";
        LOG(WARNING)<<err;
        throw InternalServerException(err);
    }
    return Document(doc_json_str);
}

std::vector<CollectionMeta> StorageService::get_all_collection_meta() const
{
    std::vector<std::string> meta_values;
    _storage_engine.load_with_prefix(KeyConverter::collection_meta_prefix(),meta_values);
    std::vector<CollectionMeta> collection_meta;
    for(const auto &meta_v:meta_values){
        CollectionMeta meta(meta_v);
        collection_meta.emplace_back(std::move(meta));
    }
    return collection_meta;
}

StorageEngine &StorageService::get_storage_engine()
{
    return this->_storage_engine;
}

void StorageService::scan_for_each_doc(const uint32_t collection_id, std::function<void (const std::string_view)> callback) const
{
    std::string seq_key_prefix=KeyConverter::doc_seq_key_prefix(collection_id);
    this->_storage_engine.scan_for_each(seq_key_prefix,callback);
}

bool StorageService::write_document(uint32_t collection_id,const Document &document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.get_raw().Accept(writer);

    StorageEngine::Batch batch;
    const std::string &doc_key=KeyConverter::doc_id_key(collection_id,document.get_doc_id());
    const std::string &seq_key=KeyConverter::doc_seq_key(collection_id,document.get_seq_id().value());
    const std::string &next_seq_key=KeyConverter::collection_next_seq_key(collection_id);

    batch.Put(doc_key,seq_key);
    batch.Put(seq_key,buffer.GetString());
    batch.Put(next_seq_key,std::to_string(document.get_seq_id().value()+1));
    return _storage_engine.batch_write(batch);
}

bool StorageService::write_new_collection(uint32_t colletion_id, const CollectionMeta &collection_meta)
{
    const std::string &collection_name=collection_meta.get_collection_name();
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    collection_meta.get_raw().Accept(writer);

    StorageEngine::Batch batch;
    batch.Put(KeyConverter::next_collection_id_key(),std::to_string(colletion_id+1));
    batch.Put(KeyConverter::collection_meta_key(collection_name),buffer.GetString());
    batch.Put(KeyConverter::collection_next_seq_key(colletion_id),std::to_string(0));
    return _storage_engine.batch_write(batch);
}



} //namespace storage
} //namespace skilo
