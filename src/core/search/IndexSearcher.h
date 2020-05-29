#ifndef INDEXSEARCHER_H
#define INDEXSEARCHER_H

#include "core/Document.h"
#include "core/index/Indexes.h"
#include "core/index/Tokenizer.h"
#include "core/search/HitCollector.h"

namespace Skilo {

using Index::TokenSet;

namespace Search {

class IndexSearcher
{
public:
    IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,const Index::TokenizeStrategy *tokenizer);

    std::vector<pair<uint32_t,double>> search();

    void do_search_field(const std::string &field_name,TokenSet token_set,HitCollector &hit_collector);


    std::vector<string> get_fuzzy_term(const TokenSet &token_set,const string &term,const string &field_name,const size_t distance) const;

private:
    size_t max_edit_distance_allowed(const std::string &term) const;


    template<typename T,typename F1,typename F2>
    std::vector<std::vector<std::string>> search_fuzz_term(const std::string &field_name,const std::string &term,
                        F1 iterator,F2 fuzzy_term_collector,size_t exact_prefix_len,size_t max_edit_distance) const;

    std::vector<std::vector<string>> search_en_fuzz_term(const std::string &field_name,const std::string &term,
                                                                    size_t exact_prefix_len=1,size_t max_edit_distance=2) const;

    std::vector<std::vector<string>> search_ch_fuzz_term(const std::string &field_name,const std::string &term,
                                                                    size_t exact_prefix_len=0,size_t max_edit_distance=1) const;

private:
    // num of different terms's cost combination
    // e.g term A,term B, term C with edit distance[1,0,2] is one combination
    static constexpr size_t _max_cost_comb=10;

    // num of different terms with given cost combination
    static constexpr size_t _max_term_comb=10;

    const Query &_query_info;
    const Index::CollectionIndexes &_indexes;
    const Index::TokenizeStrategy *_tokenizer;
};

} //namespace Search
} //namespace Skilo

#endif // INDEXSEARCHER_H
