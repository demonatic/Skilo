#include "StorageService.h"
#include "KeyConverter.hpp"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace Skilo {
namespace Storage{

StorageService::StorageService(const std::string &db_path):_storage_engine(db_path)
{

}

bool StorageService::contain_collection(const std::string &collection_name)
{
    const std::string collection_meta_key=KeyConverter::collection_meta_key(collection_name);
    return _storage_engine.contains(collection_meta_key);
}

std::optional<uint32_t> StorageService::get_collection_next_seq_id(const std::string &collection_name) const
{
    std::string next_seq_key;
    StorageEngine::Status next_seq_status=_storage_engine.get(KeyConverter::collection_next_seq_key(collection_name),next_seq_key);
    if(next_seq_status==StorageEngine::ERROR){
        return std::nullopt;
    }
    uint32_t next_seq_id=std::stoul(next_seq_key);
    return std::make_optional<uint32_t>(next_seq_id);
}

Document StorageService::get_document(const uint32_t collection_id,const std::string &collection_name,const uint32_t seq_id) const
{
    const std::string &seq_key=KeyConverter::doc_seq_key(collection_id,seq_id);
    std::string doc_json_str;
    _storage_engine.get(seq_key,doc_json_str);
    return Document(collection_name,doc_json_str);
}

bool StorageService::write_document(uint32_t collection_id,const Document &document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.get_raw().Accept(writer);

    StorageEngine::Batch batch;
    const std::string &doc_key=KeyConverter::doc_id_key(collection_id,document.get_doc_id());
    const std::string &seq_key=KeyConverter::doc_seq_key(collection_id,document.get_seq_id().value());
    const std::string &next_seq_key=KeyConverter::collection_next_seq_key(document.get_collection_name());

    batch.put(doc_key,seq_key);
    batch.put(seq_key,buffer.GetString());
    batch.put(next_seq_key,std::to_string(document.get_seq_id().value()+1));
    return _storage_engine.batch_write(batch);
}

bool StorageService::write_new_collection(uint32_t next_colletion_id, const CollectionMeta &collection_meta)
{
    const std::string &collection_name=collection_meta.get_collection_name();
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    collection_meta.get_raw().Accept(writer);

    StorageEngine::Batch batch;
    batch.put(KeyConverter::get_next_collection_id_key(),std::to_string(next_colletion_id));
    batch.put(KeyConverter::collection_meta_key(collection_name),buffer.GetString());
    batch.put(KeyConverter::collection_next_seq_key(collection_name),std::to_string(0));
    return _storage_engine.batch_write(batch);
}



} //namespace storage
} //namespace skilo
