#ifndef INVERTINDEX_H
#define INVERTINDEX_H

#include <unordered_map>
#include "Art.hpp"
#include "PostingList.h"
#include "../Document.h"
#include "../schema/Schema.h"

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
    uint32_t num_docs(const std::string &term) const;

    PostingList *get_postinglist(const std::string &term) const;

private:
    Art::ARTree<PostingList> _index;
};

class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const Schema::CollectionSchema &schema);
    virtual void visit_field_string(const Schema::FieldString *field_string) override;

    InvertIndex *get_index(const std::string &field_path);
    const InvertIndex *get_index(const std::string &field_path) const;

    bool contains(const std::string &field_path) const;

    std::vector<std::vector<std::pair<uint32_t,double>>> search_fields(const std::unordered_map<std::string, std::vector<uint32_t>> &query_terms,const std::vector<std::string> &field_paths) const;
    /// @return a vector of <doc_seq_id,score>
    std::vector<std::pair<uint32_t,double>> search_field(const std::unordered_map<std::string, std::vector<uint32_t>> &query_terms,const std::string &field_path) const;

    uint32_t num_documents(const std::string &field_path,const std::string &term) const;
private:
    std::unordered_map<std::string,InvertIndex> _indexes; //<field_path,index>
};

} //namespace Index
} //namespace Skilo
#endif // INVERTINDEX_H
