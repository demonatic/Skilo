#include "Scorer.h"
#include <cmath>
#include <iostream>
namespace Skilo {
namespace Search {

Scorer::~Scorer()
{

}

float TFIDF_Scorer::get_score(const HitContext &context) const
{
    float score=0;
    for(const Index::PostingList *posting:*context.term_postings){
        score+=this->calcu_tf_idf(posting,context);
    }
    return score;
}

float TFIDF_Scorer::calcu_tf_idf(const Index::PostingList* posting,const HitContext &context) const
{
    double tf=1+log(posting->get_doc_tf(context.doc_seq_id));
    std::cout<<"context.collection_doc_count="<<context.collection_doc_count<<" /="<<posting->num_docs()<<std::endl;
    double idf=log(2/posting->num_docs());
    return static_cast<float>(tf*idf);
}

} //namespace search
} //namespace skilo
