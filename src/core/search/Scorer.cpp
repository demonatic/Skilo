#include "Scorer.h"
#include "HitCollector.h"
#include "core/index/Indexes.h"
#include <cmath>

namespace Skilo {
namespace Search {

Scorer::~Scorer()
{

}

number_t TextScorer::get_score(const MatchContext &context) const
{
    double score=0;
    for(size_t i=0;i<context.term_postings.size();i++){
        double term_score=this->calcu_term_score(context.term_postings[i],context);
        term_score=apply_phrase_match_boost(term_score,context);
        score+=apply_term_cost_penalty(term_score,context.token_costs[i]);
    }
    return number_t(apply_total_cost_penalty(score,context.token_costs)*context.boost);
}

double TextScorer::apply_term_cost_penalty(double score, const size_t cost) const
{
    assert(cost<=_max_term_cost);
    return score*(1<<(4-cost));
}


double TextScorer::apply_total_cost_penalty(double score,const std::vector<size_t> &costs) const
{
    size_t total_cost=std::accumulate(costs.begin(),costs.end(),0);
    size_t max_cost=costs.size()<<2;
    return score*(max_cost-total_cost)/max_cost;
}

double TextScorer::apply_phrase_match_boost(double score, const MatchContext &context) const
{
    return score*(1+log(1+context.phrase_match_count*context.query_terms.size()));
}

double TFIDF_Scorer::calcu_term_score(const Index::PostingList* posting,const MatchContext &context) const
{
    double tf=1+log(posting->get_doc_tf(context.doc_seq_id));
    double idf=log(context.collection_doc_count/posting->num_docs());
    return tf*idf;
}

double BM25_Scorer::calcu_term_score(const Index::PostingList *posting, const MatchContext &context) const
{
    uint32_t tf=posting->get_doc_tf(context.doc_seq_id);
    uint32_t doc_len=posting->get_doc_len(context.doc_seq_id);
    double R=(k1*tf)/(tf+k1*(1-b+b*doc_len/posting->avg_doc_len()));
    uint32_t df=posting->num_docs();
    double W=log(context.collection_doc_count-df+0.5)/(df+0.5);
    return W*R;
}

number_t SortScorer::get_score(const MatchContext &context) const
{
    const Index::SortIndex &sort_index=context.sort_indexes.at(_rank_field);
    number_t number=sort_index.get_numeric_val(context.doc_seq_id);
    if(_is_ascend){
        number=-number;
    }
    return number;
}

} //namespace search
} //namespace skilo
