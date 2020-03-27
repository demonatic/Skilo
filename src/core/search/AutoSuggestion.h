#ifndef AUTOSUGGESTION_H
#define AUTOSUGGESTION_H

#include "core/index/Art.hpp"
#include "parallel_hashmap/phmap.h"
#include <queue>

namespace Skilo {
namespace Search{

/// @class store the top k hot search content of each prefix
/// and provide the user with top k hot search content of given input as prefix
class AutoSuggestor
{
public:
    /// @param max query len is byte length rather than unicode length
    AutoSuggestor(const size_t suggestion_num=5,const size_t min_gram=2,const size_t max_gram=15,const size_t max_query_len=45);

    /// @brief update the most hot search content of query's each prefix using edge-ngram
    void update(const std::string &query);

    /// @brief get top k hot queries with given prefix
    std::vector<std::string_view> auto_suggest(const std::string &query_prefix);

private:
    std::vector<std::string_view> edge_ngram(const std::string_view query);
    size_t get_character_len(const char c);

private:

    struct SuggestNode{
        std::string query;
        size_t freq;
        bool operator<(const SuggestNode &other){
            return this->freq<other.freq;
        }
    };
    using SuggestionList=std::vector<SuggestNode>; //ascend according to frequency

    size_t _suggestion_num;
    size_t _max_query_len;
    size_t _min_gram,_max_gram;

    // <query_prefix,SuggestionList>
    phmap::parallel_flat_hash_map<std::string,SuggestionList> _suggest_map;
    // <query,frequency>
    phmap::parallel_flat_hash_map<std::string,size_t> _query_freq_map;
};

} //namespace Search
} //namespace Skilo


#endif // AUTOSUGGESTION_H
