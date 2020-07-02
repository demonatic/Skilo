#ifndef COLLECTIONMANAGER_H
#define COLLECTIONMANAGER_H

#include "Collection.h"
#include "libcuckoo/cuckoohash_map.hh"
#include <atomic>

namespace Skilo {

using ResultStr=std::string;

class CollectionManager
{
public:
    CollectionManager(const SkiloConfig &config);

    void init_collections();

    ResultStr overall_summary() const;

    ResultStr collection_summary(const std::string &collection_name) const;

    ResultStr create_collection(CollectionMeta &collection_meta);

    ResultStr add_document(const std::string &collection_name,Document &document);

    /// @param 'seq' indicate whether the 'id' is seq id, if not, use doc id
    ResultStr get_document(const std::string &collection_name,const uint32_t id,bool seq=false);

    ResultStr remove_document(const std::string &collection_name,const uint32_t doc_id);

    ResultStr add_document_batch(const std::string &collection_name,DocumentBatch &doc_batch);

    ResultStr drop_collection(const std::string &collection_name);

    ResultStr search(const Query &query_info) const;
    
    ResultStr auto_suggest(const std::string &collection_name,const std::string &query_prefix) const;

    std::shared_ptr<Collection> get_collection(const std::string &collection_name) const;

private:
    uint32_t get_next_collection_id();

private:
    const SkiloConfig &_config;

    std::atomic_uint32_t _next_collection_id;

    cuckoohash_map<std::string,uint32_t> _collection_name_id_map;
    cuckoohash_map<uint32_t,std::shared_ptr<Collection>> _collection_map;

    std::unique_ptr<StorageService> _storage_service;

};

} //namespace Skilo

#endif // COLLECTIONMANAGER_H
