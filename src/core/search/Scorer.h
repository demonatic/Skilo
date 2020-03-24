#ifndef SCORER_H
#define SCORER_H

#include <vector>
#include <string>
#include "utility/Number.h"
#include "core/index/PostingList.h"

namespace Skilo {
namespace Search {

struct HitContext;

class Scorer
{
public:
    Scorer()=default;
    virtual ~Scorer();
    virtual number_t get_score(const HitContext &context) const=0;
};

class TFIDF_Scorer:public Scorer{
public:
    TFIDF_Scorer()=default;
    virtual number_t get_score(const HitContext &context) const;

    double calcu_tf_idf(const Index::PostingList* posting,const HitContext &context) const;
};

class BM25_Scorer:public Scorer{
public:
    virtual number_t get_score(const HitContext &context) const;
    double calcu_term_score(const Index::PostingList* posting,const HitContext &context) const;

private:
    double k1=1.2;
    double k3=1;
    double b=0.75;
};

class SortScorer:public Scorer{
public:
    using IsAscendOrder=bool;

    SortScorer(const std::string &rank_field,IsAscendOrder ascend_order)
        :_is_ascend(ascend_order),_rank_field(rank_field)
    {

    }
    virtual number_t get_score(const HitContext &context) const;

private:
    IsAscendOrder _is_ascend;
    std::string _rank_field;
};

} //namespace search
} //namespace skilo

#endif // SCORER_H
