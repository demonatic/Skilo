#include "HitCollector.h"
#include <iostream>
namespace Skilo  {
namespace Search {

HitCollector::HitCollector(const size_t K,std::unique_ptr<Scorer> scorer):
    _K(K),_hits(_K+1),_scorer(std::move(scorer)),_heap_index(0),_min_score_heap(_K+1,nullptr)
{
    for(size_t i=1;i<=_K;i++){
        _min_score_heap[i]=_hits.data()+i;
    }
}

std::vector<pair<uint32_t,float>> HitCollector::get_top_k()
{
    uint32_t docs_collected=this->num_docs_collected();
    std::vector<pair<uint32_t,float>> top_k_docs(docs_collected);
    for(int64_t i=docs_collected-1;i>=0;i--){
        top_k_docs[i]={this->top().doc_seq_id,this->top().score};
        this->pop_least_score_doc();
    }
    _doc_id_map.clear();
    return top_k_docs;
}

void HitCollector::collect(const HitContext &context)
{
    uint32_t doc_id=context.doc_seq_id;
    float score=_scorer->get_score(context);
    if(this->num_docs_collected()>=_K&&score<=this->top().score){
        return;
    }

    auto it=_doc_id_map.find(doc_id);
    if(it!=_doc_id_map.end()){
        //update the already exists doc's score(higher) and sift heap
        Hit *hit=it->second;
        if(score>hit->score){
            hit->score=score;
            heap_sift_down(hit->heap_index);
        }
    }
    else{
        Hit new_hit{score,doc_id,0};
        if(num_docs_collected()>=_K){
           uint32_t victim_id=this->pop_least_score_doc();
           _doc_id_map.erase(victim_id);
        }
        this->push_new_hit(new_hit);
    }
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
        if(_min_score_heap[left_index]->score<_min_score_heap[min_index]->score){
            min_index=left_index;
        }
        if(right_index<=_heap_index&&_min_score_heap[right_index]->score<_min_score_heap[min_index]->score){
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
    swap((*hit1)->heap_index,(*hit2)->heap_index);
    swap(*hit1,*hit2);
}

void HitCollector::push_new_hit(HitCollector::Hit &hit)
{
    hit.heap_index=++this->_heap_index;
    _doc_id_map[hit.doc_seq_id]=_min_score_heap[_heap_index];
    *_min_score_heap[_heap_index]=hit;

    size_t cur_index=_heap_index;
    while(cur_index>1){
        size_t parent_index=cur_index/2;
        if(_min_score_heap[parent_index]->score<=_min_score_heap[cur_index]->score){
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

Scorer *HitCollector::get_scorer() const
{
    return _scorer.get();
}

bool HitCollector::empty() const
{
    return _heap_index==0;
}

} //namespace search
} //namespace skilo
