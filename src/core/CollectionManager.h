#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

#include "Collection.h"
#include "parallel_hashmap/phmap.h"

namespace Skilo {

struct Status;
class CollectionManager
{
public:
    CollectionManager(const std::string &db_path);

    /// @return error string if error exists
    Status create_collection(const std::string &collection_name,CollectionMeta &collection_meta);

    Status add_document(const std::string &collection_name,const Document &document);

private:
    uint32_t get_next_collection_id();

private:
    std::unordered_map<std::string,uint32_t> _collection_name_id_map;
    std::unordered_map<uint32_t,std::unique_ptr<Collection>> _collection_map;

    std::unique_ptr<StorageService> _storage_service;

    uint32_t _next_collection_id;
};

enum class RetCode{
    OK=200,
    CREATED=201,
    NOT_CONTENT=204,
    BAD_REQUEST=400,
    FORBIDDEN=403,
    NOT_FOUND=404,
    METHOD_NOT_ALLOWED=405,
    CONFLICT=409,
    INTERNAL_SERVER_ERROR=500
};

struct Status{
    RetCode code;
    std::string info;
};

} //namespace Skilo

#endif // COLLECTIONMANAGER_H
