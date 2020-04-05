#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "CompressedScalar.hpp"
#include <vector>

namespace Skilo {
namespace Index{

/// @class containing a list of 'document(id) and the corresponding info(Term Frequency,Posision,Offset)'
/// The postlist represents a document in sequence id
class PostingList
{
public:
    PostingList();

    uint32_t num_docs() const;
    double avg_doc_len() const;

    /// @note offsets must be in ascending order
    void add_doc(const uint32_t seq_id,const uint32_t doc_len,const std::vector<uint32_t> &offsets);

    uint32_t get_doc_id(const uint32_t index) const;
    uint32_t get_doc_tf(const uint32_t doc_id) const;
    uint32_t get_doc_len(const uint32_t doc_id) const;
    std::vector<uint32_t> get_doc_term_offsets(const uint32_t doc_id) const;

    bool contain_doc(const uint32_t id) const;
    std::unique_ptr<uint32_t[]> get_all_doc_ids() const;

private:
    double _avg_doc_len;

    CompressedScalar<ScalarType::Sorted> _doc_ids;
    CompressedScalar<ScalarType::UnSorted> _doc_term_freqs;

    CompressedScalar<ScalarType::UnSorted> _doc_len;

    CompressedScalar<ScalarType::UnSorted> _offsets;
    CompressedScalar<ScalarType::Sorted> _offset_index;
};

} //namespace Index
} //namespace Skilo
#endif // POSTINGLIST_H
