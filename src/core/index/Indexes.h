#ifndef INDEXES_H
#define INDEXES_H

#include <unordered_map>
#include "Art.hpp"
#include "PostingList.h"
#include "core/Document.h"
#include "core/schema/Schema.h"
#include "core/index/Tokenizer.h"
#include "core/search/AutoSuggestion.h"
#include "core/search/MatchContext.hpp"
#include "storage/StorageService.h"
#include "utility/RWLock.hpp"
#include "utility/Number.h"
#include "parallel_hashmap/phmap.h"


namespace Skilo {

namespace Index{

struct StrRecord{
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

    void remove_number(uint32_t seq_id);
};

class InvertIndex
{
public:
    InvertIndex();

    void index_str_record(const StrRecord &record);
    void remove_str_record(const StrRecord &record);

    bool contain_term(const std::string &term) const;
    size_t dict_size() const;

    void debug_print_dict() const;

    /// @brief return how many doc contain this term
    uint32_t term_docs_num(const std::string &term) const;

    void search_field(const std::string &field_path,const std::unordered_map<string,std::vector<uint32_t>> &token_to_offsets,const std::vector<size_t> &costs,size_t slop,uint32_t total_doc_count,const std::unordered_map<std::string, SortIndex> *sort_indexes,std::function<void(Search::MatchContext&)> on_match) const;

    void iterate_terms(const std::string &prefix,std::function<void(unsigned char *,size_t,PostingList*)> on_term,
                 std::function<bool(unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const;

    void iterate_pinyin(const std::string &prefix,std::function<void(unsigned char *,size_t,std::unordered_set<std::string>*)> on_pinyin,
                        std::function<bool(unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const;

private:
    PostingList *get_postinglist(const std::string &term) const;

private:
    //indexed term->posting_list
    mutable RWLock _term_posting_lock;
    Art::ARTree<PostingList> _term_postings;

    //pinyin->terms, to support chinese fuzzy search
    mutable RWLock _pinyin_terms_lock;
    Art::ARTree<std::unordered_set<std::string>> _pinyin_terms;
};

class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const uint32_t collection_id, const Schema::CollectionSchema &schema,
                      const CollectionMeta &collection_meta,const Storage::StorageService *storage_service);

    InvertIndex *get_invert_index(const std::string &field_path);
    const InvertIndex *get_invert_index(const std::string &field_path) const;

    Search::AutoSuggestor *get_suggestor() const;

    bool contains(const std::string &field_path) const;

    void set_doc_num(const uint32_t doc_num);

    uint32_t get_doc_num() const;

    uint32_t field_term_doc_num(const std::string &field_path,const std::string &term) const;

    void index_numeric(const std::string &field_path,const uint32_t seq_id,const number_t number);
    void remove_numeric(const std::string &field_path,const uint32_t seq_id);

    void search_fields(const std::string &field_path,const std::unordered_map<string,std::vector<uint32_t>> &token_to_offsets,const std::vector<size_t> &costs,size_t slop,std::function<void(Search::MatchContext&)> on_match) const;

protected:
    virtual void visit_field_string(const Schema::FieldString *field_string) override;
    virtual void visit_field_integer(const Schema::FieldInteger *field_integer) override;
    virtual void visit_field_float(const Schema::FieldFloat *field_float) override;

private:
    void init_sort_field(const Schema::Field *field_numeric);

private:
    uint32_t _collection_id;
    std::atomic_uint32_t _doc_num;
    const Storage::StorageService *_storage_service;

    //!-- <field_path,index>
    std::unordered_map<std::string,InvertIndex> _indexes;
    //!-- <field_path,<doc_id,numeric_value>>
    std::unordered_map<std::string,SortIndex> _sort_indexes;

    std::unique_ptr<Search::AutoSuggestor> _auto_suggestor;
};


} //namespace Index
} //namespace Skilo
#endif // INDEXES_H
