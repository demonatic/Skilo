#ifndef INVERTINDEX_H
#define INVERTINDEX_H

#include "Art.hpp"
#include "PostingList.h"
#include "../Document.h"
#include "../Schema.h"

namespace Skilo {
namespace Index{

struct IndexRecord{
    uint32_t seq_id;
    std::vector<std::string_view> terms;
};

class InvertIndex
{
public:
    InvertIndex();
    void add_record(const IndexRecord &record);

private:
    struct TermEntry{
        uint32_t _doc_freq; //the number of docs include the term
        PostingList _posting_list;
    };
    Art::ARTree<TermEntry> _index;
};


} //namespace Index
} //namespace Skilo
#endif // INVERTINDEX_H
