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

    uint32_t get_id() const;
    const std::string& get_name() const;

    /// @brief read collection's documents from storage and index them
    void build_index();

    /// @brief destroy all info about this collection including memory indexes and in storage
    void drop_all();

    /// @brief validate the document and then index the required field
    /// @throw InvalidFormatException, InternalServerException
    void add_new_document(Document &document);

    void remove_document(const uint32_t doc_id);

    bool contain_document(const uint32_t doc_id) const;

    /// @brief check whether document adhere to the schema
    /// @throw InvalidFormatException
    void validate_document(const Document &document);

    /// @return phrase search
    /// @throw InternalServerException
    SearchResult search(const Query &query_info) const;

    uint32_t get_doc_num() const;
    
    std::vector<std::string_view> auto_suggest(const std::string &query_prefix) const;
    
private:
    std::unique_ptr<Index::TokenizeStrategy> get_tokenize_strategy(const std::string &tokenizer_name) const;

private:
    const uint32_t _collection_id;
    const std::string _collection_name;

    std::atomic_uint32_t _next_seq_id;
    std::atomic_uint32_t _doc_num;

    StorageService *_storage_service;

    Schema::CollectionSchema _schema;
    Index::CollectionIndexes _indexes;

    std::unique_ptr<Index::TokenizeStrategy> _tokenizer;

    const SkiloConfig &_config;
};


} //namespace Skilo

#endif // COLLECTION_H


