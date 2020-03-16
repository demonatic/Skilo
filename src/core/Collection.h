#ifndef COLLECTION_H
#define COLLECTION_H

#include "Document.h"
#include "schema/Schema.h"
#include "index/IndexWriter.h"
#include "storage/StorageService.h"
#include "SkiloConfig.h"
#include <string>
#include <optional>
#include <atomic>

namespace Skilo {

using Storage::StorageService;

class Collection
{
public:
    Collection(const CollectionMeta &collection_meta,StorageService *storage_service,const SkiloConfig &config);

    /// @brief read collection's documents from storage and index them
    void build_index();

    /// @brief validate the document and then index the required field
    /// @throw InvalidFormatException, InternalServerException
    void add_new_document(Document &document);

    bool contain_document(const uint32_t doc_id) const;

    /// @brief check whether document adhere to the schema
    /// @throw InvalidFormatException
    void validate_document(const Document &document);

    /// @return phrase search
    /// @throw InternalServerException
    SearchResult search(const Query &query_info) const;

    uint32_t document_num() const;

    std::unique_ptr<Index::TokenizeStrategy> get_tokenize_strategy(const std::string &tokenizer_name) const;

private:
    std::atomic_uint32_t _collection_id;
    std::atomic_uint32_t _next_seq_id;
    std::string _collection_name;

    StorageService *_storage_service;

    Schema::CollectionSchema _schema;
    Index::CollectionIndexes _indexes;

    std::unique_ptr<Index::TokenizeStrategy> _tokenizer;

    const SkiloConfig &_config;
};


} //namespace Skilo

#endif // COLLECTION_H


