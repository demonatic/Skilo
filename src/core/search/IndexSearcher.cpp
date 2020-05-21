#include "IndexSearcher.h"
#include "utility/CodeTiming.h"
#include "rocapinyin.h"

using namespace Skilo::Index;

namespace Skilo {
namespace Search {


IndexSearcher::IndexSearcher(const Query &query_info, const Index::CollectionIndexes &indexes,
                             const Index::TokenizeStrategy *tokenizer)
    :_query_info(query_info),_indexes(indexes),_tokenizer(tokenizer)
{

}

std::vector<pair<uint32_t,double>> IndexSearcher::do_search()
{
    //extract and split query terms and fields
    const std::string &query_str=_query_info.get_search_str();
    std::unordered_map<std::string, std::vector<uint32_t>> query_terms=_tokenizer->tokenize(query_str);

    const vector<std::string> &search_fields=_query_info.get_query_fields();

    std::vector<Token> tokens;
    for(auto &&[term,offsets]:query_terms){
        std::cout<<"term="<<term<<std::endl;
        Token token;
        token.term=term;
        token.offsets=offsets;
        token.fuzzy_terms=search_ch_fuzz_term(search_fields[0],term,0,2);
//        timing_code(token.fuzzy_terms=search_en_fuzz_term(search_fields[0],term,1,2));
        size_t distance=0;
        for(auto v:token.fuzzy_terms){
            std::cout<<"---distance="<<distance<<"----"<<std::endl;
            for(auto s:v){
                std::cout<<s<<" ";
            }
            distance++;
            std::cout<<std::endl;
        }
    }

    //init search criteria and do search
    uint32_t top_k=50;
    Search::DocRanker ranker;
    auto &sort_fields=_query_info.get_sort_fields();
    if(sort_fields.empty()){
        ranker.push_scorer(std::make_unique<Search::BM25_Scorer>());
    }
    else{
        for(auto &&[sort_field,ascend_order]:sort_fields){
            //TODO test field exists
            ranker.push_scorer(std::make_unique<Search::SortScorer>(sort_field,ascend_order));
        }
    }

    Search::HitCollector collector(top_k,ranker);
    _indexes.search_fields(query_terms,search_fields,collector);

    AutoSuggestor *suggestor=_indexes.get_suggestor();
    if(suggestor){
        suggestor->update(query_str);
    }
    //collect hit documents
    return collector.get_top_k();
}

std::vector<std::vector<string>> IndexSearcher::search_en_fuzz_term(const string &field_name, const string &term, size_t exact_prefix_len, size_t max_edit_distance)
{
    auto en_fuzzy_term_collector=[](std::vector<std::string> &terms,unsigned char *fuzz_str, size_t len,PostingList *){
        terms.emplace_back(std::string(fuzz_str,fuzz_str+len));
    };
    auto term_iterator=[](const InvertIndex *index){
        return std::bind(&InvertIndex::iterate_terms,index,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
    };
    return search_fuzz_term<PostingList>(field_name,term,exact_prefix_len,max_edit_distance,en_fuzzy_term_collector,term_iterator);
}

std::vector<std::vector<string> > IndexSearcher::search_ch_fuzz_term(const string &field_name, const string &term, size_t exact_prefix_len, size_t max_edit_distance)
{
    std::string term_pinyin=rocapinyin::getpinyin_str(term);
    Util::trim(term_pinyin,' ');
    std::cout<<"search term pinyin="<<term_pinyin<<std::endl;
    auto ch_fuzzy_term_collector=[](std::vector<std::string> &terms,unsigned char *, size_t,std::unordered_set<std::string> *term_set){
        for(auto &&term:*term_set){
            terms.push_back(term);
        }
    };
    auto pinyin_iterator=[](const InvertIndex *index){
        return std::bind(&InvertIndex::iterate_pinyin,index,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4);
    };
    return search_fuzz_term<std::unordered_set<std::string>>(field_name,term_pinyin,exact_prefix_len,max_edit_distance,ch_fuzzy_term_collector,pinyin_iterator);
}

template<typename T,typename F1,typename F2>
std::vector<std::vector<std::string>> IndexSearcher::search_fuzz_term(const std::string &field_name,const string &term,
        size_t exact_prefix_len,size_t max_edit_distance,F1 fuzzy_term_collector,F2 iterator) const
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
    std::string test;
    distance_table.emplace_back(std::move(first_row));

    size_t push=0,pop=0;
    auto on_fuzzy_match=[&](unsigned char *str, size_t len, T* t){
        std::string match=std::string(str,str+len);
        size_t distance=static_cast<size_t>(distance_table.back().back());
        if(distance<=max_edit_distance){
            fuzzy_term_collector(fuzzy_matches[distance],str,len,t);
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
        test.push_back(c);
        return false;
    };
    auto on_backtrace=[&](unsigned char c){
        if(c!='\0'){
             distance_table.pop_back();
             test.pop_back();
        }
    };

    //iterate though the whole term dict
    //it's O(n) operation but incremental calculation along with early termination will save a lot of time
    iterator(invert_index)(exact_match_prefix,on_fuzzy_match,early_termination,on_backtrace);
    return fuzzy_matches;
}



} //namespace Search
} //namespace Skilo
