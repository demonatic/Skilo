#include "IndexSearcher.h"

namespace Skilo {
namespace Search {


IndexSearcher::IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,
                             const Index::TokenizeStrategy *tokenizer)
    :_query_info(query_info),_indexes(indexes),_tokenizer(tokenizer)
{

}

std::vector<pair<uint32_t,double>> IndexSearcher::do_search()
{
    //extract and split query terms and fields
    const std::string &query_str=_query_info.get_search_str();
    std::unordered_map<std::string, std::vector<uint32_t>> query_terms=_tokenizer->tokenize(query_str);
    std::vector<Token> result_tokens; //TODO init it

    const vector<std::string> &search_fields=_query_info.get_query_fields();

    //init search criteria and do search
    uint32_t top_k=50;
    Search::DocRanker ranker;
    auto &sort_fields=_query_info.get_sort_fields();
    if(sort_fields.empty()){
        ranker.push_scorer(std::make_unique<Search::BM25_Scorer>());
    }
    else{
        for(auto &&[sort_field,ascend_order]:sort_fields){
            //TODO test field exists
            ranker.push_scorer(std::make_unique<Search::SortScorer>(sort_field,ascend_order));
        }
    }

    Search::HitCollector collector(top_k,ranker);
    _indexes.search_fields(query_terms,search_fields,collector);

    AutoSuggestor *suggestor=_indexes.get_suggestor();
    if(suggestor){
        suggestor->update(query_str);
    }
    //collect hit documents
    return collector.get_top_k();
}

} //namespace Search
} //namespace Skilo
