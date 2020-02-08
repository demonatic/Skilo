#ifndef INVERTINDEX_H
#define INVERTINDEX_H

#include <unordered_map>
#include "Art.hpp"
#include "PostingList.h"
#include "../Document.h"
#include "../schema/Schema.h"
#include "../search/HitCollector.h"

namespace Skilo {
namespace Index{

struct IndexRecord{
    uint32_t seq_id;
    std::unordered_map<std::string, std::vector<uint32_t>> term_offsets;
};

class InvertIndex
{
public:
    InvertIndex();
    void add_record(const IndexRecord &record);

    size_t dict_size() const;
    uint32_t term_docs_num(const std::string &term) const;

    PostingList *get_postinglist(const std::string &term) const;

private:
    Art::ARTree<PostingList> _index;
};

class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const Schema::CollectionSchema &schema);

    InvertIndex *get_index(const std::string &field_path);
    const InvertIndex *get_index(const std::string &field_path) const;
    bool contains(const std::string &field_path) const;
    uint32_t field_term_doc_num(const std::string &field_path,const std::string &term) const;

    void set_doc_num(const uint32_t doc_num);
    uint32_t get_doc_num() const;

    void search_fields(const std::unordered_map<std::string, std::vector<uint32_t>> &query_terms,const std::vector<std::string> &field_paths,Search::HitCollector &collector) const;
    void search_field(const std::unordered_map<std::string, std::vector<uint32_t>> &query_terms,const std::string &field_path,Search::HitCollector &collector) const;

protected:
    virtual void visit_field_string(const Schema::FieldString *field_string) override;

private:
    uint32_t _doc_count;
    std::unordered_map<std::string,InvertIndex> _indexes; //<field_path,index>

};

} //namespace Index
} //namespace Skilo
#endif // INVERTINDEX_H
