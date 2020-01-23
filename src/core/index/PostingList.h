#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "CompressedScalar.hpp"
#include <roaring/roaring.hh>

namespace Skilo {
namespace Index{

/// @class containing a list of 'document(id) and the corresponding info(Term Frequency,Posision,Offset)'
class PostingList
{
public:
    PostingList();

private:
    CompressedScalar<ScalarType::Sorted> _doc_ids;
    CompressedScalar<ScalarType::Sorted> _doc_freqs;
};

} //namespace Index
} //namespace Skilo
#endif // POSTINGLIST_H
