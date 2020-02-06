#ifndef HITCOLLECTOR_H
#define HITCOLLECTOR_H

#include <vector>
#include <queue>
#include <memory>
#include "Scorer.h"
#include "parallel_hashmap/phmap.h"

namespace Skilo {
namespace Search {

class HitCollector{
    template<typename _Tp, typename _Sequence = vector<_Tp>,
         typename _Compare  = less<typename _Sequence::value_type> >
    class PriorityQueue:public std::priority_queue<_Tp,_Sequence,_Compare>{
    public:
        PriorityQueue(const _Compare& __x):std::priority_queue<_Tp,_Sequence,_Compare>(__x){}
        _Sequence get_container(){
            return this->c;
        }
    };

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
    PriorityQueue<QueueItem,std::vector<QueueItem>,decltype(cmp)> _priority_queue; //min-heap
    phmap::node_hash_map<uint32_t,float> _doc_scores;
};


} //namespace search
} //namespace skilo

#endif // HITCOLLECTOR_H
