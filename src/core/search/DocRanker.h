#ifndef DOCRANKER_H
#define DOCRANKER_H

#include <list>
#include "Scorer.h"

namespace Skilo {
namespace Search {

class DocRanker
{
public:
    DocRanker();

    void push_scorer(std::unique_ptr<Scorer> scorer);

    std::vector<number_t> rank(const HitContext &ctx) const;

    size_t rank_criteria_num() const;

    inline static constexpr size_t MaxRankCriteriaCount=4;
private:
    size_t _scorer_num;
    std::array<std::unique_ptr<Scorer>,MaxRankCriteriaCount> _scorers;
};

} //namespace search
} //namespace skilo


#endif // DOCRANKER_H
