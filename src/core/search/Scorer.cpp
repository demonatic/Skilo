#include "Scorer.h"
#include <cmath>

namespace Skilo {
namespace Search {

Scorer::~Scorer()
{

}

float TFIDF_Scorer::get_score(const HitContext &context) const
{
    float score=0;
    for(const Index::PostingList *posting:*context.term_postings){
        score+=context.phrase_match_count*this->calcu_tf_idf(posting,context);
    }
    return score;
}

float TFIDF_Scorer::calcu_tf_idf(const Index::PostingList* posting,const HitContext &context) const
{
    double tf=1+log(posting->get_doc_tf(context.doc_seq_id));
    double idf=log(context.collection_doc_count/posting->num_docs());
    return static_cast<float>(tf*idf);
}

} //namespace search
} //namespace skilo
