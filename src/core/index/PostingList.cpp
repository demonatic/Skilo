#include "PostingList.h"
#include <iostream>
namespace Skilo {
namespace Index{

PostingList::PostingList():_avg_doc_len(0)
{

}

uint32_t PostingList::num_docs() const
{
    return _doc_ids.length();
}

double PostingList::avg_doc_len() const
{
    return _avg_doc_len;
}

void PostingList::add_doc(const uint32_t seq_id,const uint32_t doc_len,const std::vector<uint32_t> &offsets)
{
    _avg_doc_len=(_avg_doc_len*num_docs()+doc_len)/(num_docs()+1);
    _doc_len.append(doc_len);

    _doc_ids.append(seq_id);
     uint32_t term_freq=offsets.size();
    _doc_term_freqs.append(term_freq);

    uint32_t curr_offset_index=_offsets.length();
    _offset_index.append(curr_offset_index);
    for(uint32_t off:offsets){
        _offsets.append(off);
    }
}

uint32_t PostingList::get_doc_id(const uint32_t index) const
{
    return _doc_ids[index];
}

uint32_t PostingList::get_doc_tf(const uint32_t doc_id) const
{
    uint32_t index=_doc_ids.index_of(doc_id);
    return _doc_term_freqs[index];
}

uint32_t PostingList::get_doc_len(const uint32_t doc_id) const
{
    uint32_t index=_doc_ids.index_of(doc_id);
    return _doc_len[index];
}

std::vector<uint32_t> PostingList::get_doc_term_offsets(const uint32_t doc_id) const
{
    uint32_t pos=_doc_ids.index_of(doc_id);
    uint32_t offset_start=_offset_index[pos];
    uint32_t offset_end=(pos!=_offset_index.length()-1)?_offset_index[pos+1]:_offsets.length();
    vector<uint32_t> offsets;
    for(uint32_t i=offset_start;i<offset_end;i++){
        offsets.push_back(_offsets[i]);
    }
    return offsets;
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
