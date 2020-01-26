#include "PostingList.h"

namespace Skilo {
namespace Index{

PostingList::PostingList()
{

}

void PostingList::add_doc(uint32_t seq_id)
{
    _doc_ids.append(seq_id);
}

std::vector<uint32_t> PostingList::get_common_doc_id(const PostingList &other)
{

}

} //namespace Index
} //namespace Skilo
