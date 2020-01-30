#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

#include "Collection.h"
#include "../../3rd/include/parallel_hashmap/phmap.h"

namespace Skilo {

class CollectionManager
{
public:
    CollectionManager(const std::string &db_path);

    /// @return error string if error exists
    std::optional<std::string> create_collection(const std::string &collection_name,CollectionMeta &collection_meta);

    std::optional<std::string> add_document(uint32_t collection_name,const Document &document);

private:
    uint32_t get_next_collection_id();

private:
    std::unordered_map<std::string,uint32_t> _collection_name_id_map;
    std::unordered_map<uint32_t,std::unique_ptr<Collection>> _collection_map;

    std::unique_ptr<StorageService> _storage_service;

    uint32_t _next_collection_id;
};

} //namespace Skilo

#endif // COLLECTIONMANAGER_H
