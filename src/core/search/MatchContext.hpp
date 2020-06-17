#ifndef HITCONTEXT_HPP
#define HITCONTEXT_HPP


#include <vector>
#include <string>
#include <unordered_map>

namespace Skilo {
namespace Index {
    class SortIndex;
    class PostingList;
}
namespace Search {

struct MatchContext{
    uint32_t doc_seq_id;
    uint32_t collection_doc_count;
    uint32_t phrase_match_count;
    float boost;
    const std::string &field_path;
    const std::vector<const Index::PostingList*> &term_postings;
    const std::vector<size_t> &token_costs;
    const std::unordered_map<std::string,std::vector<uint32_t>> &query_terms;
    const std::unordered_map<std::string,Index::SortIndex> &sort_indexes;
};

}
}

#endif // HITCONTEXT_HPP
