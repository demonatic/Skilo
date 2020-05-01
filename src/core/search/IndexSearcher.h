#ifndef INDEXSEARCHER_H
#define INDEXSEARCHER_H

#include "../index/Indexes.h"
#include "../index/Tokenizer.h"
#include "../Document.h"

namespace Skilo {
namespace Search {

class IndexSearcher
{
public:
    IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,const Index::TokenizeStrategy *tokenizer);
    std::vector<pair<uint32_t,double>> do_search();

private:
    const Query &_query_info;
    const Index::CollectionIndexes &_indexes;
    const Index::TokenizeStrategy *_tokenizer;
};

} //namespace Search
} //namespace Skilo

#endif // INDEXSEARCHER_H
