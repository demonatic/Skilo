#include "DocRanker.h"
#include <iostream>

namespace Skilo {
namespace Search {

DocRanker::DocRanker():_scorer_num(0)
{

}

void DocRanker::push_scorer(std::unique_ptr<Scorer> scorer)
{
    if(_scorer_num==MaxRankCriteriaCount){
        return;
    }
    _scorers[_scorer_num++]=std::move(scorer);
}

std::vector<number_t> DocRanker::rank(const HitContext &ctx) const
{
    std::vector<number_t> rank_scores;
    for(size_t i=0;i<_scorer_num;i++){
        number_t score=_scorers[i]->get_score(ctx);
        rank_scores.push_back(score);
    }
    return rank_scores;
}

size_t DocRanker::rank_criteria_num() const
{
    return _scorer_num;
}

} //namespace search
} //namespace skilo
