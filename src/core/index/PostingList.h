#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "CompressedScalar.hpp"
#include <roaring/roaring.hh>

namespace Skilo {
namespace Index{

/// @class containing a list of 'document(id) and the corresponding info(Term Frequency,Posision,Offset)'
/// The postlist represents a document in sequence id
class PostingList
{
public:
    PostingList();
    uint32_t num_docs() const;
    void add_doc(const uint32_t seq_id,const std::vector<uint32_t> &offsets);

    uint32_t get_doc_id(const uint32_t index) const;
    bool contain_doc(const uint32_t id) const;
    std::unique_ptr<uint32_t[]> get_all_doc_ids() const;

private:
    CompressedScalar<ScalarType::Sorted> _doc_ids;
    CompressedScalar<ScalarType::Sorted> _doc_term_freqs;
};

} //namespace Index
} //namespace Skilo
#endif // POSTINGLIST_H
