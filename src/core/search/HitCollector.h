#ifndef HITCOLLECTOR_H
#define HITCOLLECTOR_H

#include <vector>
#include <memory>
#include "Scorer.h"

namespace Skilo {
namespace Search {

/// @class HitCollecter collects search candidate from different fields and rank them based on scorer generated score
class HitCollector{

public:
    HitCollector(const size_t K,std::unique_ptr<Scorer> scorer);

    /// @brief get top K score document sequence ids in score descending order
    /// the collector will be empty after the call
    std::vector<pair<uint32_t,float>> get_top_k();

    /// @note when we have the same doc id already and the newer score is greater, we have to update the min-heap
    void collect(const HitContext &context);

    bool empty() const;
    size_t num_docs_collected() const;
    Scorer *get_scorer() const;

private:
    struct Hit{
        float score;
        uint32_t doc_seq_id;
        size_t heap_index=0;
    };

    Hit &top();

    void heap_sift_down(size_t index);
    void swap_entry(Hit **hit1,Hit **hit2);
    uint32_t pop_least_score_doc();
    void push_new_hit(Hit &hit);

private:
    const size_t _K;
    std::vector<Hit> _hits; //pre-allocated space for heap entry
    std::unique_ptr<Scorer> _scorer;

    size_t _heap_index;
    std::vector<Hit*> _min_score_heap; //points to _hits
    std::unordered_map<uint32_t,Hit*> _doc_id_map;
};


} //namespace search
} //namespace skilo

#endif // HITCOLLECTOR_H