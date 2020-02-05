#ifndef SCORER_H
#define SCORER_H

#include <vector>
#include <string>
#include "../index/PostingList.h"

namespace Skilo {
namespace Search {

struct HitContext{
    uint32_t doc_seq_id;
    std::string field_path;
    uint32_t field_doc_num;
    std::vector<const Index::PostingList*> _hit_postings;
};

class Scorer
{
public:
    Scorer()=default;
    virtual ~Scorer();
    virtual float get_score(const HitContext &context) const=0;
};

class TFIDF_Scorer:public Scorer{
public:
    TFIDF_Scorer()=default;
    virtual float get_score(const HitContext &context) const;
};

} //namespace search
} //namespace skilo

#endif // SCORER_H
