#include "HitCollector.h"

namespace Skilo  {
namespace Search {

HitCollector::HitCollector(const size_t K,std::unique_ptr<Scorer> scorer):_K(K),_scorer(std::move(scorer)),_priority_queue(cmp)
{

}

std::vector<uint32_t> HitCollector::get_top_k()
{
    uint32_t docs_collected=static_cast<uint32_t>(_priority_queue.size());
    std::vector<uint32_t> top_k_docs(docs_collected,0);
    for(int64_t i=docs_collected-1;i>=0;i--){
        top_k_docs[i]=_priority_queue.top().doc_seq_id;
        _priority_queue.pop();
    }
    return top_k_docs;
}

void HitCollector::collect(const HitContext &context)
{
    float score=_scorer->get_score(context);
    if(_priority_queue.size()==_K&&score<_priority_queue.top().score){
        return;
    }
    QueueItem new_entry{score,context.doc_seq_id};
    _priority_queue.push(new_entry);
}



} //namespace search
}//namespace skilo
