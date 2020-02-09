#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <optional>
#include "Document.h"
#include "schema/Schema.h"
#include "index/IndexWriter.h"
#include "../storage/StorageService.h"

namespace Skilo {

using Storage::StorageService;

class Collection
{
public:
    Collection(const CollectionMeta &collection_meta,StorageService *storage_service);

    /// @brief validate the document and then index the required field
    /// @throw InvalidFormatException, InternalServerException
    void add_document(Document &document);

    /// @brief check whether document adhere to the schema
    /// @throw InvalidFormatException
    void validate_document(const Document &document);

    /// @return phrase search
    /// @throw InternalServerException
    SearchResult search(const Query &query_info) const;

    uint32_t document_num() const;
    std::unique_ptr<Index::TokenizeStrategy> get_tokenize_strategy(const std::string &tokenizer_name) const;

private:
    uint32_t _collection_id;
    uint32_t _next_seq_id;
    std::string _collection_name;

    Schema::CollectionSchema _schema;
    Index::CollectionIndexes _indexes;

    std::unique_ptr<Index::TokenizeStrategy> _tokenizer;

    StorageService *_storage_service;
};


} //namespace Skilo

#endif // COLLECTION_H


