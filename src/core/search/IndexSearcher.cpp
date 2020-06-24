#include "IndexSearcher.h"
#include "utility/Util.h"
#include "rocapinyin.h"
#include "g3log/g3log.hpp"
#include <string_view>

using namespace Skilo::Index;

namespace Skilo {
namespace Search {


IndexSearcher::IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,
                             const Index::TokenizeStrategy *tokenizer)
    :_query_info(query_info),_indexes(indexes),_tokenizer(tokenizer)
{

}

void IndexSearcher::search(std::vector<std::pair<uint32_t,double>> &res_docs)
{
    //extract and split query terms and fields
    const std::string &query_str=_query_info.get_search_str();
    TokenSet token_set=_tokenizer->tokenize(query_str);

    //init rank criteria
    uint32_t top_k=50;
    Search::DocRanker ranker;

    for(auto &&[sort_field,ascend_order]:_query_info.get_sort_fields()){
        if(!_indexes.contains(sort_field)){
            throw InvalidFormatException("sort field \""+sort_field+"\" not exists");
        }
        ranker.add_scorer(std::make_unique<Search::SortScorer>(sort_field,ascend_order));
    }
    ranker.add_scorer(std::make_unique<Search::BM25_Scorer>());

    Search::HitCollector collector(top_k,ranker);

    //search each field
    const std::vector<std::string> &query_fields=_query_info.get_query_fields();
    for(size_t i=0;i<query_fields.size();i++){
        const std::string &field=query_fields[i];
        if(!_indexes.contains(field)){
            throw InvalidFormatException("query field \""+field+"\" not exists or not indexed");
        }
        do_search_field(field,token_set,_query_info.get_field_boosts()[i],collector);
    }

    //get top K
    res_docs=collector.get_top_k();
}

std::vector<std::string>& IndexSearcher::get_fuzzy_term(const TokenSet &token_set, const std::string &term, const std::string &field_name, const size_t distance) const
{
    return token_set.get_fuzzies(term,distance,[&,this](const size_t distance){
        return Util::is_chinese(term)?
            this->search_ch_fuzz_term(field_name,term,0,distance):
            this->search_en_fuzz_term(field_name,term,1,distance);
    });
}

void IndexSearcher::do_search_field(const std::string &field_name,TokenSet token_set,float boost,HitCollector &hit_collector)
{
    if(token_set.empty())
        return;

    LOG(DEBUG)<<"search field:\""<<field_name<<"\" token:\""<<token_set.to_string()<<"\"";
    const InvertIndex *index=_indexes.get_invert_index(field_name);

    size_t slop=0;
    size_t query_len_total=0;
    std::vector<string> token_names;
    std::vector<std::vector<size_t>> token_allowing_costs;

    for(auto &&[term,offsets]:token_set.term_to_offsets()){
        query_len_total+=term.length();
        token_names.push_back(term);
        size_t max_allow_cost=this->max_edit_distance_allowed(term);
        std::vector<size_t> costs(max_allow_cost+1);
        for(size_t i=0;i<=max_allow_cost;i++){
            costs[i]=i;
        }
        token_allowing_costs.emplace_back(std::move(costs));
    }

    slop=2*query_len_total/token_set.term_to_offsets().size();

    std::unordered_map<std::string,size_t> token_count_sum;

    // when generate a combination of cost(edit distance) for each query token
    // e.g edit distance [2,0,1] from origin three query token
    auto on_cost_comb=[&](const std::vector<size_t> &costs){
        std::vector<std::vector<std::string>> candidate_tokens; //each token's fuzzy terms with given cost
        LOG(DEBUG)<<"generate costs="<<costs;
        for(size_t i=0;i<costs.size();i++){
            std::vector<std::string> &fuzzies=this->get_fuzzy_term(token_set,token_names[i],field_name,costs[i]);
            if(fuzzies.empty()){ //the i'th token doesn't have any match with edit distance costs[i]
                return;
            }
            //sort fuzzy terms according to term frequency
            std::sort(fuzzies.begin(),fuzzies.end(),[=](const std::string &fuzzy_termA,const std::string &fuzzy_termB){
                return index->term_docs_num(fuzzy_termA)>index->term_docs_num(fuzzy_termB);
            });
            candidate_tokens.push_back(fuzzies);
            LOG(DEBUG)<<"cost combination="<<costs<<" fuzzies="<<fuzzies;
        }
        //given each token's cost, generate every possible token
        Util::cartesian(candidate_tokens,[&,this](const std::vector<std::string> &query_term){
            assert(query_term.size()==token_names.size());

            //token(exact or fuzzy)->offsets in query string
            std::unordered_map<string,std::vector<uint32_t>> token_to_offsets;
            for(size_t i=0;i<query_term.size();i++){
                std::vector<uint32_t> offsets=token_set.get_offsets(token_names[i]);
                token_to_offsets.emplace(query_term[i],std::move(offsets));
            }
            LOG(DEBUG)<<"query term="<<query_term<<" costs="<<costs;
            _indexes.search_fields(field_name,token_to_offsets,costs,slop,[&](Search::MatchContext &match_ctx){
                match_ctx.boost=boost;
                hit_collector.collect(match_ctx);
            });
        },_max_term_comb);
    };

    Util::cartesian(token_allowing_costs,on_cost_comb,_max_cost_comb);

    // check if num of result collected is too small, if so, calculate the token(along with it's fuzzy terms) frequency,
    // drop the least occuring token and search again
    if(hit_collector.num_docs_collected()<hit_collector.get_k()/2){
        for(const auto &token_name:token_names){
            token_count_sum[token_name]=0;
        }
        for(size_t i=0;i<token_allowing_costs.size();i++){
            for(int cost:token_allowing_costs[i]){
                for(auto &fuzzy_term:get_fuzzy_term(token_set,token_names[i],field_name,cost)){
                    token_count_sum[token_names[i]]+=index->term_docs_num(fuzzy_term);
                }
            }
        }

        std::sort(token_names.begin(),token_names.end(),[&](const std::string &termA,const std::string &termB){
            return token_count_sum[termA]<token_count_sum[termB];
        });

        LOG(DEBUG)<<"drop token "<<token_names[0];
        token_set.drop_token(token_names[0]);
        do_search_field(field_name,token_set,boost,hit_collector);
    }
}

std::vector<std::vector<std::string>> IndexSearcher::search_en_fuzz_term(const std::string &field_name, const std::string &term, size_t exact_prefix_len, size_t max_edit_distance) const
{
    auto en_fuzzy_term_collector=[](std::vector<std::vector<std::string>> &fuzzy_matches,size_t distance,unsigned char *fuzz_str, size_t len,PostingList *){
        fuzzy_matches[distance].emplace_back(std::string(fuzz_str,fuzz_str+len));
    };
    auto term_iterator=[](const InvertIndex *index){
        return std::bind(&InvertIndex::iterate_terms,index,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
    };
    return search_fuzz_term<PostingList>(field_name,term,term_iterator,en_fuzzy_term_collector,exact_prefix_len,max_edit_distance);
}

std::vector<std::vector<string>> IndexSearcher::search_ch_fuzz_term(const std::string &field_name, const std::string &term, size_t exact_prefix_len, size_t max_edit_distance) const
{
    std::string term_pinyin=rocapinyin::getpinyin_str(term);
    Util::trim(term_pinyin,' ');
    auto ch_fuzzy_term_collector=[&](std::vector<std::vector<std::string>> &fuzzy_matches,size_t distance,unsigned char *, size_t,std::unordered_set<std::string> *term_set){
        if(distance+1>max_edit_distance){
            return;
        }
        if(distance==0&&term_set->count(term)){
            fuzzy_matches[0].push_back(term);
            term_set->erase(term);
        }
        for(auto &&term:*term_set){
            fuzzy_matches[distance+1].push_back(term);
        }
    };
    auto pinyin_iterator=[](const InvertIndex *index){
        return std::bind(&InvertIndex::iterate_pinyin,index,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
    };
    return search_fuzz_term<std::unordered_set<std::string>>(field_name,term_pinyin,pinyin_iterator,ch_fuzzy_term_collector,exact_prefix_len,max_edit_distance);
}

size_t IndexSearcher::max_edit_distance_allowed(const std::string &term) const
{
    size_t len=Util::utf8_len(term);
    size_t max_allow_dis=0;
    if(len<=3){
        max_allow_dis=0;
    }
    else if(len<=5){
        max_allow_dis=1;
    }
    else{
        max_allow_dis=2;
    }
    return max_allow_dis;
}

template<typename T,typename F1,typename F2>
std::vector<std::vector<std::string>> IndexSearcher::search_fuzz_term(const std::string &field_name,const std::string &term,
                                                                      F1 iterator,F2 fuzzy_term_collector,size_t exact_prefix_len,size_t max_edit_distance) const
{
    const InvertIndex *invert_index=_indexes.get_invert_index(field_name);
    if(!invert_index)
        return{};

    std::vector<std::vector<std::string>> fuzzy_matches(max_edit_distance+1); //index: edit distance, elm: fuzzy matches
    size_t term_len=term.length();
    std::string exact_match_prefix=term.substr(0,exact_prefix_len);

    std::vector<std::vector<int>> distance_table; //dp, contains an empty row and col
    int last_row_min_dis=0;
    std::vector<int> first_row(term_len+1);
    //init first row
    for(size_t i=0;i<=term_len;i++){
        first_row[i]=i;
    }

    distance_table.emplace_back(std::move(first_row));

    auto on_fuzzy_node=[&](unsigned char *str, size_t len, T* t){
        std::string match=std::string(str,str+len);
        size_t distance=static_cast<size_t>(distance_table.back().back());
        if(distance<=max_edit_distance){
            fuzzy_term_collector(fuzzy_matches,distance,str,len,t);
        }
    };
    auto early_termination=[&](unsigned char c){
        if(c=='\0'){
            return false;
        }
        std::vector<int> &prev_row=distance_table.back();
        std::vector<int> curr_row(term_len+1);
        curr_row[0]=prev_row[0]+1;
        last_row_min_dis=curr_row[0];
        for(size_t i=1;i<=term_len;i++){
            int insert_cost=curr_row[i-1]+1;
            int delete_cost=prev_row[i]+1;
            int replace_cost=term[i-1]!=c?prev_row[i-1]+1:prev_row[i-1];
            curr_row[i]=std::min({insert_cost,delete_cost,replace_cost});
            last_row_min_dis=std::min(curr_row[i],last_row_min_dis);
        }
        //if current row's min distance is greater than max_distance, we stop traversing downwards
        if(last_row_min_dis>max_edit_distance){
            return true;
        }
        distance_table.emplace_back(std::move(curr_row));
        return false;
    };
    auto on_backtrace=[&](unsigned char c){
        if(c!='\0'){
             distance_table.pop_back();
        }
    };

    //iterate though the whole term dict
    //it's O(n) operation but incremental calculation along with early termination will save a lot of time
    iterator(invert_index)(exact_match_prefix,on_fuzzy_node,early_termination,on_backtrace);
    return fuzzy_matches;
}




} //namespace Search
} //namespace Skilo
