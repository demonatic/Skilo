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
    std::unordered_map<std::string, std::vector<uint32_t>> word_offsets;
};

class InvertIndex
{
public:
    InvertIndex();
    void add_record(const IndexRecord &record);

private:
    struct TermEntry{
        uint32_t doc_freq; //the number of docs include the term
        PostingList posting_list;
    };
    Art::ARTree<TermEntry> _index;
};

class CollectionIndexes:public Schema::FieldVisitor{
public:
    CollectionIndexes(const Schema::CollectionSchema &schema);
    virtual void visit_field_string(const Schema::FieldString *field_string) override;

    InvertIndex &get_index(const std::string &field_path);
    bool contains(const std::string &field_path) const;
private:
    std::unordered_map<std::string,InvertIndex> _indexes; //<field_path,index>
};

} //namespace Index
} //namespace Skilo
#endif // INVERTINDEX_H
