#include "Scorer.h"
#include <cmath>
#include "HitCollector.h"
#include "core/index/Indexes.h"

namespace Skilo {
namespace Search {

Scorer::~Scorer()
{

}

number_t TFIDF_Scorer::get_score(const HitContext &context) const
{
    double score=0;
    for(const Index::PostingList *posting:*context.term_postings){
        score+=context.phrase_match_count*this->calcu_tf_idf(posting,context);
    }
    return number_t(score);
}

double TFIDF_Scorer::calcu_tf_idf(const Index::PostingList* posting,const HitContext &context) const
{
    double tf=1+log(posting->get_doc_tf(context.doc_seq_id));
    double idf=log(context.collection_doc_count/posting->num_docs());
    return tf*idf;
}

number_t SortScorer::get_score(const HitContext &context) const
{
    const Index::SortIndex &sort_index=context.sort_indexes->at(_rank_field);
    number_t number=sort_index.get_numeric_val(context.doc_seq_id);
    if(_is_ascend){
        number=-number;
    }
    return number;
}

number_t BM25_Scorer::get_score(const HitContext &context) const
{
    double bm25_score=0;
    for(const Index::PostingList *posting:*context.term_postings){
        bm25_score+=this->calcu_term_score(posting,context);
    }
    return number_t(bm25_score);
}

double BM25_Scorer::calcu_term_score(const Index::PostingList *posting, const HitContext &context) const
{
    uint32_t tf=posting->get_doc_tf(context.doc_seq_id);
    uint32_t doc_len=posting->get_doc_len(context.doc_seq_id);
    double R=(k1*tf)/(tf+k1*(1-b+b*doc_len/posting->avg_doc_len()));
    uint32_t df=posting->num_docs();
    double W=log(context.collection_doc_count-df+0.5)/(df+0.5);
    return W*R;
}

} //namespace search
} //namespace skilo
