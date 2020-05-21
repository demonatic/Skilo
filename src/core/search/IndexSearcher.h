#ifndef INDEXSEARCHER_H
#define INDEXSEARCHER_H

#include "core/index/Indexes.h"
#include "core/index/Tokenizer.h"
#include "core/Document.h"

namespace Skilo {
namespace Search {

struct Token{
    std::string term;
    std::vector<uint32_t> offsets;
    std::vector<std::vector<std::string>> fuzzy_terms; //index: edit distance, element: fuzzy matches
};

class IndexSearcher
{
public:
    IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,const Index::TokenizeStrategy *tokenizer);

    std::vector<pair<uint32_t,double>> do_search();

    std::vector<std::vector<std::string>> search_en_fuzz_term(const std::string &field_name,const std::string &term,
                                                                    size_t exact_prefix_len=1,size_t max_edit_distance=2);

    std::vector<std::vector<std::string>> search_ch_fuzz_term(const std::string &field_name,const std::string &term,
                                                                    size_t exact_prefix_len=0,size_t max_edit_distance=1);
private:
    template<typename T,typename F1,typename F2>
    std::vector<std::vector<std::string>> search_fuzz_term(const std::string &field_name,const std::string &term,
                            size_t exact_prefix_len,size_t max_edit_distance,F1 get_fuzzy_term,F2 iterator) const;

private:
    const Query &_query_info;
    const Index::CollectionIndexes &_indexes;
    const Index::TokenizeStrategy *_tokenizer;
};

} //namespace Search
} //namespace Skilo

#endif // INDEXSEARCHER_H
