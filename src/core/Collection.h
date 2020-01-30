#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <optional>
#include "Document.h"
#include "schema/Schema.h"
#include "../storage/StorageService.h"

namespace Skilo {

using Storage::StorageService;

class Collection
{
public:
    Collection(const CollectionMeta &collection_meta,StorageService *storage_service);
    bool index_document(const Document &document);

    /// @brief check whether document adhere to the schema
    bool validate_document(const Document &document);
    
private:
    uint32_t _collection_id;
    uint32_t _next_seq_id;
    std::string _collection_name;

    Schema::CollectionSchema _schema;
    StorageService *_storage_service;
};


} //namespace Skilo

#endif // COLLECTION_H


