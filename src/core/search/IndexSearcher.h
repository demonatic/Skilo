#ifndef INDEXSEARCHER_H
#define INDEXSEARCHER_H

#include "../index/Indexes.h"
#include "../index/Tokenizer.h"
#include "../Document.h"

namespace Skilo {
namespace Search {

struct Token{
    size_t offset;
    std::string content;
    std::vector<std::vector<std::string>> fuzzy_tokens_array; // edit distance->token contents with given distance
};

class IndexSearcher
{
public:
    IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,const Index::TokenizeStrategy *tokenizer);
    std::vector<pair<uint32_t,double>> do_search();

    void edit_distance() const;

    void on_within_distance(const std::string &pinyin,const size_t edit_distance,const std::vector<std::string> &fuzzy_tokens);

private:
    const Query &_query_info;
    const Index::CollectionIndexes &_indexes;
    const Index::TokenizeStrategy *_tokenizer;
};

} //namespace Search
} //namespace Skilo

#endif // INDEXSEARCHER_H
