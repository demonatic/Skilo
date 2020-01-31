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
    std::vector<std::string_view> tokens;
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
private:
    std::unordered_map<std::string,InvertIndex> _indexes; //<field_path,index>
};

} //namespace Index
} //namespace Skilo
#endif // INVERTINDEX_H
