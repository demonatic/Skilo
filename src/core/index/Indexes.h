#ifndef INDEXES_H
#define INDEXES_H

#include <unordered_map>
#include "Art.hpp"
#include "PostingList.h"
#include "core/Document.h"
#include "core/schema/Schema.h"
#include "core/search/HitCollector.h"
#include "storage/StorageService.h"
#include "utility/RWLock.hpp"
#include "utility/Number.h"
#include "parallel_hashmap/phmap.h"
#include "../search/AutoSuggestion.h"

namespace Skilo {

namespace Index{

struct IndexRecord{
    uint32_t seq_id;
    uint32_t doc_len;
    std::unordered_map<std::string, std::vector<uint32_t>> term_offsets;
};

struct SortIndex{
    bool cache;
    uint32_t collection_id;
    std::string field_path;
    mutable phmap::parallel_flat_hash_map<uint32_t,number_t> index;
    Storage::StorageService *storage;

    number_t get_numeric_val(const uint32_t doc_seq_id) const;
    void add_number(uint32_t seq_id,const number_t number);
};


class Indexes
{
public:
    Indexes();
    void add_record(const IndexRecord &record);

    size_t dict_size() const;

    uint32_t term_docs_num(const std::string &term) const;

    void search_field(const std::unordered_map<string, std::vector<uint32_t>> &query_terms,
        const std::string &field_path,Search::HitCollector &collector,uint32_t total_doc_count,
            const std::unordered_map<string, SortIndex> *sort_indexes) const;

private:
    PostingList *get_postinglist(const std::string &term) const;

private:
    mutable RWLock _index_lock;
    Art::ARTree<PostingList> _index;
};

class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const uint32_t collection_id, const Schema::CollectionSchema &schema,
                      const CollectionMeta &collection_meta,const Storage::StorageService *storage_service);

    Indexes *get_invert_index(const std::string &field_path);
    const Indexes *get_invert_index(const std::string &field_path) const;

    Search::AutoSuggestor *get_suggestor() const;

    bool contains(const std::string &field_path) const;
    uint32_t field_term_doc_num(const std::string &field_path,const std::string &term) const;

    void set_doc_num(const uint32_t doc_num);
    uint32_t get_doc_num() const;

    void search_fields(const std::unordered_map<std::string, std::vector<uint32_t>> &query_terms,
                       const std::vector<std::string> &field_paths,Search::HitCollector &collector) const;

    void add_sort_field(const std::string &field_path,const uint32_t seq_id,const number_t number);

protected:
    virtual void visit_field_string(const Schema::FieldString *field_string) override;
    virtual void visit_field_integer(const Schema::FieldInteger *field_integer) override;
    virtual void visit_field_float(const Schema::FieldFloat *field_float) override;

private:
    void init_sort_field(const Schema::Field *field_numeric);

private:
    uint32_t _doc_count;
    uint32_t _collection_id;
    const Storage::StorageService *_storage_service;

    //!-- <field_path,index>
    std::unordered_map<std::string,Indexes> _indexes;
    //!-- <field_path,<doc_id,numeric_value>>
    std::unordered_map<std::string,SortIndex> _sort_indexes;

    std::unique_ptr<Search::AutoSuggestor> _auto_suggestor;
};


} //namespace Index
} //namespace Skilo
#endif // INDEXES_H