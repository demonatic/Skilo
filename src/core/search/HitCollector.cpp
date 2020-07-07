#include "HitCollector.h"

namespace Skilo  {
namespace Search {

HitCollector::HitCollector(const size_t K,DocRanker &ranker):
    _K(K),_num_docs_checked(0),_hits(_K+1),_ranker(std::move(ranker)),_heap_index(0),_min_score_heap(_K+1,nullptr)
{
    for(size_t i=1;i<=_K;i++){
        _min_score_heap[i]=_hits.data()+i;
    }
}

std::vector<pair<uint32_t,double>> HitCollector::get_top_k()
{
    int64_t docs_collected=this->num_docs_collected();
    std::vector<pair<uint32_t,double>> top_k_docs(docs_collected);
    for(int64_t i=docs_collected-1;i>=0;i--){
        double score=this->top().rank_scores[0].real();
        top_k_docs[i]={this->top().doc_seq_id,score};
        this->pop_least_score_doc();
    }
    _doc_id_map.clear();
    return top_k_docs;
}

void HitCollector::collect(const MatchContext &context)
{
    uint32_t doc_id=context.doc_seq_id;

    std::vector<number_t> scores=_ranker.rank(context);
    if(this->num_docs_collected()>=_K&&scores<=this->top().rank_scores){
        return;
    }

    auto it=_doc_id_map.find(doc_id);
    if(it!=_doc_id_map.end()){
        //update the already exists doc's score(higher) and sift heap
        Hit *hit=it->second;
        if(scores>hit->rank_scores){
            hit->rank_scores=std::move(scores);
            heap_sift_down(hit->heap_index);
        }
    }
    else{
        Hit new_hit{doc_id,0,scores};
        if(num_docs_collected()>=_K){
           uint32_t victim_id=this->pop_least_score_doc();
           _doc_id_map.erase(victim_id);
        }
        this->push_new_hit(new_hit);
    }
    _num_docs_checked++;
}

HitCollector::Hit &HitCollector::top()
{
    return *_min_score_heap[1];
}

uint32_t HitCollector::pop_least_score_doc()
{
    uint32_t victim_id=this->top().doc_seq_id;
    swap_entry(&_min_score_heap[1],&_min_score_heap[_heap_index--]);
    heap_sift_down(1);
    return victim_id;
}

void HitCollector::heap_sift_down(size_t index)
{
    size_t cur_index=index;
    while(cur_index<=_heap_index/2){
        size_t left_index=cur_index*2,right_index=left_index+1;
        size_t min_index=cur_index;
        if(_min_score_heap[left_index]->rank_scores<_min_score_heap[min_index]->rank_scores){
            min_index=left_index;
        }
        if(right_index<=_heap_index&&_min_score_heap[right_index]->rank_scores<_min_score_heap[min_index]->rank_scores){
            min_index=right_index;
        }
        if(min_index==cur_index){
            break;
        }
        swap_entry(&_min_score_heap[cur_index],&_min_score_heap[min_index]);
        cur_index=min_index;
    }
}

void HitCollector::swap_entry(Hit **hit1,Hit **hit2)
{
    std::swap((*hit1)->heap_index,(*hit2)->heap_index);
    std::swap(*hit1,*hit2);
}

void HitCollector::push_new_hit(HitCollector::Hit &hit)
{
    hit.heap_index=++this->_heap_index;
    _doc_id_map[hit.doc_seq_id]=_min_score_heap[_heap_index];
    *_min_score_heap[_heap_index]=hit;

    size_t cur_index=_heap_index;
    while(cur_index>1){
        size_t parent_index=cur_index/2;
        if(_min_score_heap[parent_index]->rank_scores<=_min_score_heap[cur_index]->rank_scores){
            break;
        }
        swap_entry(&_min_score_heap[parent_index],&_min_score_heap[cur_index]);
        cur_index=parent_index;
    }
}

uint32_t HitCollector::num_docs_collected() const
{
    return static_cast<uint32_t>(_heap_index);
}

uint32_t HitCollector::num_docs_checked() const{
    return _num_docs_checked;
}

uint32_t HitCollector::get_k() const
{
    return _K;
}

DocRanker& HitCollector::get_ranker()
{
    return _ranker;
}

bool HitCollector::empty() const
{
    return _heap_index==0;
}

} //namespace search
} //namespace skilo
