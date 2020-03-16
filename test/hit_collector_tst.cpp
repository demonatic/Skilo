#include <gtest/gtest.h>
#include <unordered_set>
#include "utility/Number.h"
#include "core/search/HitCollector.h"

using namespace testing;
using namespace std;
using namespace Skilo::Search;
using namespace Skilo;

struct TestScorer:public Scorer{
public:
    virtual number_t get_score(const HitContext &context) const override{
        float score=(float)(rand()%10000)/(rand()%100+1);
        if(scores.count(context.doc_seq_id)){
            float pre_score=scores[context.doc_seq_id];
            scores[context.doc_seq_id]=max(score,pre_score);
        }
        else{
            scores[context.doc_seq_id]=score;
        }
        return score;
    }
    mutable unordered_map<uint32_t,float> scores ;
};

TEST(HIT_COLLECTOR_TEST,COLLECT_TEST) {
    size_t n=100;
    std::vector<HitContext> contexts(n);
    for(int i=0;i<n;i++){
        contexts[i].doc_seq_id=rand()%(n/2);
    }
    DocRanker ranker;
    std::unique_ptr<TestScorer> p_scorer=std::make_unique<TestScorer>();
    TestScorer *scorer=p_scorer.get();
    ranker.push_scorer(std::move(p_scorer));
    HitCollector collector(n/3,ranker);

    for(int i=0;i<n;i++){
        collector.collect(contexts[i]);
    }

    unordered_map<uint32_t,float> scores_map=scorer->scores;
    vector<pair<uint32_t,float>> data;
    for(auto pair:scores_map){
        data.push_back(pair);
    }
    sort(data.begin(),data.end(),[](auto p1,auto p2){
        return p1.second>p2.second;
    });

    vector<pair<uint32_t,double>> top_k=collector.get_top_k();

    for(int i=0;i<top_k.size();i++){
//        cout<<"data doc id="<<data[i].first<<" score="<<data[i].second<<endl;
//        cout<<"topk: i="<<i<<" id="<<top_k[i].first<<" score="<<top_k[i].second<<endl;
        EXPECT_EQ(top_k[i].first,data[i].first);
        EXPECT_EQ(top_k[i].second,data[i].second);
    }
}





