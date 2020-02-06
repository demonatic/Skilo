#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include "StorageEngine.h"
#include "../core/Document.h"

namespace Skilo {
namespace Storage{

class StorageService
{
public:
    StorageService(const std::string &db_path);
    bool contain_collection(const std::string &collection_name);
    std::optional<uint32_t> get_collection_next_seq_id(const std::string &collection_name) const;

    Document get_document(const uint32_t collection_id,const std::string &collection_name,const uint32_t seq_id) const;

public:
    /// @brief write <doc_id,seq_id>,<seq_id,doc>,<collection_next_seq_id_key,collection_next_seq_id_value> to storage
    bool write_document(uint32_t collection_id,const Document &document);
    bool write_new_collection(uint32_t next_colletion_id,const CollectionMeta &collection_meta);

private:
    StorageEngine _storage_engine;
};


} //namespace storage
} //namespace skilo

#endif // STORAGESERVICE_H
