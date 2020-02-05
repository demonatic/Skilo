#ifndef HITCOLLECTOR_H
#define HITCOLLECTOR_H

#include <vector>
#include <queue>
#include <memory>
#include "Scorer.h"

namespace Skilo {
namespace Search {

class HitCollector{

public:
    HitCollector(const size_t K,std::unique_ptr<Scorer> scorer);

    /// @brief get top K score document sequence ids in score descending order
    std::vector<uint32_t> get_top_k();
    void collect(const HitContext &context);

private:
    const size_t _K;
    std::unique_ptr<Scorer> _scorer;
    struct QueueItem{
        float score;
        uint32_t doc_seq_id;
    };
    inline static constexpr auto cmp=[](const QueueItem &item1,const QueueItem &item2){
        return item1.score>item2.score;
    };
    std::priority_queue<QueueItem,std::vector<QueueItem>,decltype(cmp)> _priority_queue; //min-heap

};


} //namespace search
} //namespace skilo

#endif // HITCOLLECTOR_H
