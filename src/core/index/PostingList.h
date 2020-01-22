#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "CompressableArray.h"
#include <roaring/roaring.hh>

/// @class containing a list of 'document(id) and the corresponding info(Term Frequency,Posision,Offset)'
class PostingList
{
public:
    PostingList();

private:
    Roaring _doc_ids;
    CompressableArray<uint32_t,true> _doc_freqs;
};

#endif // POSTINGLIST_H
