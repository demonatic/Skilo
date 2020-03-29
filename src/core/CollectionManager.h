#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

#include "Collection.h"
#include "libcuckoo/cuckoohash_map.hh"
#include <atomic>

namespace Skilo {

struct Status;
class CollectionManager
{
public:
    CollectionManager(const SkiloConfig &config);

    void init_collections();
    std::string create_collection(CollectionMeta &collection_meta);

    std::string add_document(const std::string &collection_name,Document &document);
    std::string add_document_batch(const std::string &collection_name,DocumentBatch &doc_batch);

    std::string search(const Query &query_info) const;
    
    std::string auto_suggest(const std::string &collection_name,const std::string &query_prefix) const;

    Collection *get_collection(const std::string &collection_name) const;

private:
    uint32_t get_next_collection_id();

private:
    const SkiloConfig &_config;

    cuckoohash_map<std::string,uint32_t> _collection_name_id_map;
    cuckoohash_map<uint32_t,std::unique_ptr<Collection>> _collection_map;

    std::unique_ptr<StorageService> _storage_service;

    std::atomic_uint32_t _next_collection_id;
};

} //namespace Skilo

#endif // COLLECTIONMANAGER_H
