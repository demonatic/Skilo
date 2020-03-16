#include "Scorer.h"
#include <cmath>
#include "HitCollector.h"
#include "core/index/InvertIndex.h"

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
    const Index::SortFieldProxy &sort_index=context.sort_indexes->at(_rank_field);
    number_t number=sort_index.get_numeric_val(context.doc_seq_id);
    if(_is_ascend){
        number=-number;
    }
    return number;
}

} //namespace search
} //namespace skilo
