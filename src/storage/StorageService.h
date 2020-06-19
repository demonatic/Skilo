#ifndef STORAGESERVICE_H
#define STORAGESERVICE_H

#include "StorageEngine.h"
#include "core/Document.h"

namespace Skilo {
namespace Storage{

class StorageService
{
public:
    StorageService(const std::string &db_path);

    /// @note use collection_name to conveniently check existance when trying creating new collection
    bool contain_collection(const std::string &collection_name) const;

    bool contain_document(const uint32_t collection_id,uint32_t doc_id) const;

    /// @throw InternalServerException
    uint32_t get_next_collection_id() const;

    /// @throw InternalServerException
    uint32_t get_collection_next_seq_id(uint32_t collection_id) const;

    uint32_t get_doc_seq_id(const uint32_t collection_id,const uint32_t doc_id);

    /// @brief read storage, fecth the document in the given collection and parse it
    /// @throw InternalServerException,InvalidFormatException
    Document get_document(const uint32_t collection_id,const uint32_t seq_id) const;

    /// @brief scan the storage, fecth and parse all of the collection's meta data
    /// @throw InternalServerException,InvalidFormatException
    std::vector<CollectionMeta> get_all_collection_meta() const;

    CollectionMeta get_collection_meta(const std::string &collection_name) const;

    StorageEngine &get_storage_engine();

    void scan_for_each_doc(const uint32_t collection_id,std::function<void(const std::string_view value)> callback) const;

    /// @brief delete all data about the give collection
    void drop_collection(const uint32_t collection_id,const std::string &collection_name);

    /// @brief write <doc_id,seq_id>,<seq_id,doc>,<collection_next_seq_id_key,collection_next_seq_id_value> to storage
    bool write_document(uint32_t collection_id,const Document &document);

    bool remove_document(uint32_t collection_id,const Document &document);

    bool write_new_collection(uint32_t colletion_id,const CollectionMeta &collection_meta);

private:
    StorageEngine _storage_engine;
};


} //namespace storage
} //namespace skilo

#endif // STORAGESERVICE_H
