#ifndef SCORER_H
#define SCORER_H

#include <vector>
#include <string>
#include "utility/Number.h"
#include "core/search/MatchContext.hpp"

namespace Skilo {

namespace Search {


class Scorer
{
public:
    Scorer()=default;
    virtual ~Scorer();
    virtual number_t get_score(const MatchContext &context) const=0;
};

class TextScorer:public Scorer{
public:
    virtual number_t get_score(const MatchContext &context) const override;
    virtual double calcu_term_score(const Index::PostingList* posting,const MatchContext &context) const=0;

    double apply_term_cost_penalty(double score,const size_t cost) const;
    double apply_total_cost_penalty(double score,const std::vector<size_t> &costs) const;

    double apply_phrase_match_boost(double score,const MatchContext &context) const;

private:
    static constexpr size_t _max_term_cost=4;
};

class TFIDF_Scorer:public TextScorer{
public:
    virtual double calcu_term_score(const Index::PostingList* posting,const MatchContext &context) const override;
};

class BM25_Scorer:public TextScorer{
public:
    virtual double calcu_term_score(const Index::PostingList* posting,const MatchContext &context) const override;

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
    virtual number_t get_score(const MatchContext &context) const;

private:
    IsAscendOrder _is_ascend;
    std::string _rank_field;
};

} //namespace search
} //namespace skilo

#endif // SCORER_H
