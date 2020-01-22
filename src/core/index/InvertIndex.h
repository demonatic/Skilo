#ifndef INVERTINDEX_H
#define INVERTINDEX_H

#include "Art.hpp"
#include "PostingList.h"

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

#endif // INVERTINDEX_H
