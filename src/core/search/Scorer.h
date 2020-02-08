#ifndef SCORER_H
#define SCORER_H

#include <vector>
#include <string>
#include "../index/PostingList.h"

namespace Skilo {
namespace Search {

struct HitContext{
    uint32_t doc_seq_id;
    uint32_t collection_doc_count;
    const std::string *field_path;
    const std::vector<const Index::PostingList*> *term_postings;
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

    float calcu_tf_idf(const Index::PostingList* posting,const HitContext &context) const;
};

} //namespace search
} //namespace skilo

#endif // SCORER_H
