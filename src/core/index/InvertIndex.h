#ifndef INVERTINDEX_H
#define INVERTINDEX_H

#include "Art.hpp"
#include "PostingList.h"

namespace Skilo {
namespace Index{

class InvertIndex
{
public:
    InvertIndex();
    void index_document();

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
