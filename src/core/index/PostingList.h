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
    void add_doc(uint32_t seq_id,uint32_t frequency);
    std::vector<uint32_t> get_common_doc_id(const PostingList &other);

private:
    CompressedScalar<ScalarType::Sorted> _doc_ids;
    CompressedScalar<ScalarType::Sorted> _doc_freqs;
};

} //namespace Index
} //namespace Skilo
#endif // POSTINGLIST_H
