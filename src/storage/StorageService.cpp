#include "StorageService.h"
#include "KeyConverter.hpp"
#include "../../3rd/include/rapidjson/stringbuffer.h"
#include "../../3rd/include/rapidjson/writer.h"

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


bool StorageService::write_document(uint32_t collection_id,uint32_t doc_seq_id,const Document &document)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.get_raw().Accept(writer);

    StorageEngine::Batch batch;
    const std::string doc_key=KeyConverter::doc_id_key(collection_id,document.get_doc_id());
    const std::string seq_key=KeyConverter::doc_seq_key(collection_id,doc_seq_id);
    batch.put(doc_key,seq_key);
    batch.put(seq_key,buffer.GetString());
    return _storage_engine.batch_write(batch);
}

bool StorageService::write_new_collection(uint32_t next_colletion_id, const CollectionMeta &collection_meta)
{
    const char *collection_name=collection_meta.get_collection_name();
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
