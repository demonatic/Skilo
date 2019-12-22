#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

#include "Collection.h"
#include "../../3rd/include/parallel_hashmap/phmap.h"

class CollectionManager
{
public:
    CollectionManager(const std::string &db_path);

    /// @return error string if error exists
    std::optional<std::string> create_collection(const std::string &collection_name,Schema &schema);


private:
    phmap::flat_hash_map<uint32_t,std::string> _collection_id_name_map;

    phmap::flat_hash_map<std::string,std::unique_ptr<Collection>> _collection_name_map;

    std::unique_ptr<Storage> _storage;
};

#endif // COLLECTIONMANAGER_H
