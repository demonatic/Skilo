#ifndef HITCOLLECTOR_H
#define HITCOLLECTOR_H

#include <vector>
#include <memory>
#include "Scorer.h"
#include "DocRanker.h"
#include "MatchContext.hpp"

namespace Skilo {

namespace Search {

using std::string;
using std::pair;

/// @class HitCollecter collects search candidate from different fields and rank them based on scorer generated score
///        It output top-K score's doc seq id
/// @note when the same doc id already exists and the newer score from another field is greater, we have to update the doc's score
class HitCollector{

public:
    HitCollector(const size_t K,DocRanker &ranker);

    /// @brief get top K score document sequence ids in score descending order
    /// the collector will be empty after the call
    std::vector<pair<uint32_t,double>> get_top_k();

    void collect(const MatchContext &context);

    bool empty() const;

    uint32_t get_k() const;

    uint32_t num_docs_checked() const;

    uint32_t num_docs_collected() const;

    DocRanker& get_ranker();

private:
    struct Hit{
        uint32_t doc_seq_id;
        size_t heap_index=0;
        std::vector<number_t> rank_scores;

        bool operator<(Hit &other){
            return this->rank_scores<other.rank_scores;
        }
    };

    Hit &top();
    void heap_sift_down(size_t index);
    void swap_entry(Hit **hit1,Hit **hit2);
    uint32_t pop_least_score_doc();
    void push_new_hit(Hit &hit);

private:
    const size_t _K;
    uint32_t _num_docs_checked;
    std::vector<Hit> _hits; //pre-allocated space for heap entry

    DocRanker _ranker;

    size_t _heap_index;
    ///<--points to _hits-->
    std::vector<Hit*> _min_score_heap;
    std::unordered_map<uint32_t,Hit*> _doc_id_map;
};


} //namespace search
} //namespace skilo

#endif // HITCOLLECTOR_H
