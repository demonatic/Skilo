#include "PostingList.h"

namespace Skilo {
namespace Index{

PostingList::PostingList()
{

}

uint32_t PostingList::num_docs() const
{
    return _doc_ids.length();
}

void PostingList::add_doc(const uint32_t seq_id,const std::vector<uint32_t> &offsets)
{
     uint32_t term_freq=static_cast<uint32_t>(offsets.size());
    _doc_ids.append(seq_id);
    _doc_term_freqs.append(term_freq);
}

uint32_t PostingList::get_doc_id(const uint32_t index) const
{
    return _doc_ids[index];
}

bool PostingList::contain_doc(const uint32_t id) const
{
    return _doc_ids.contains(id);
}

std::unique_ptr<uint32_t[]> PostingList::get_all_doc_ids() const
{
    return _doc_ids.uncompress();
}

} //namespace Index
} //namespace Skilo
