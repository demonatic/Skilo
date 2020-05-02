#include "AutoSuggestion.h"
#include "utility/Exception.h"

namespace Skilo {
namespace Search{

AutoSuggestor::AutoSuggestor(const size_t suggestion_num,const size_t min_gram,const size_t max_gram,const size_t max_query_len):
    _suggestion_num(suggestion_num),_max_query_len(max_query_len),_min_gram(min_gram),_max_gram(max_gram)
{

}

void AutoSuggestor::update(const std::string &query)
{
    if(query.length()>_max_query_len){
        return;
    }

    //increase query frequency
    size_t query_freq=_query_freq_map[query];
    _query_freq_map[query]=++query_freq;

    std::vector<std::string_view> edge_ngrams=this->edge_ngram(query);

    for(auto &&gram:edge_ngrams){
        auto list_it=_suggest_map.find(gram);

        if(list_it!=_suggest_map.end()){
            SuggestionList &sug=list_it->second;
            size_t sug_min_freq=sug.front().freq;
            if(sug.size()>=_suggestion_num&&query_freq<sug_min_freq){
                continue;
            }
            SuggestNode node{query,query_freq};
            size_t node_index=lower_bound(sug.begin(),sug.end(),node)-sug.begin();

            int pre_index=node_index-1;
            while(pre_index>=0&&sug[pre_index].freq==query_freq-1){ //find if query entry already exist
                if(sug[pre_index].query==query){
                    break;
                }
                --pre_index;
            }
            if(pre_index>=0&&sug[pre_index].query==query){ //query entry already in, increase its frequency by 1
                sug[pre_index].freq++;
                while(pre_index+1<sug.size()&&sug[pre_index].freq>sug[pre_index+1].freq){ //move it to right position
                    swap(sug[pre_index],sug[pre_index+1]);
                    pre_index++;
                }
            }
            else if(sug.size()<_suggestion_num){ // add as a new suggestion node
                sug.push_back({});
                size_t i=sug.size()-1;
                for(;i>node_index&&sug[i-1].freq>query_freq;i--){
                    sug[i]=sug[i-1];
                }
                sug[i]=node;
            }
            else{ //replace the first suggestion node and move it to right position
                size_t index=0;
                sug[index].freq=query_freq;
                sug[index].query=query;
                while(index+1<sug.size()&&sug[index+1].freq<=query_freq){
                    swap(sug[index],sug[index+1]);
                    index++;
                }
            }
        }
        else{ //make a new SuggestionList for this prefix
            SuggestionList sug;
            sug.push_back({query,query_freq});
            _suggest_map.emplace(gram,std::move(sug));
        }
    }
}

std::vector<std::string_view> AutoSuggestor::auto_suggest(const string &query_prefix)
{
    std::vector<std::string_view> suggestions;
    auto it=_suggest_map.find(query_prefix);
    if(it!=_suggest_map.end()){
        SuggestionList &sug=it->second;
        size_t len=sug.size();
        suggestions.resize(len);
        for(size_t i=0;i<sug.size();i++){
            suggestions[i]=sug[len-i-1].query;
        }
    }
    return suggestions;
}

size_t AutoSuggestor::get_character_len(const char c)
{
    size_t len=0;
    if((c&0x80)==0){ //0xxx xxxx -> ascii
        len=1;
    }
    else if((c&0xf8)==0xf8){ // 111110xx
        len=5;
    }
    else if((c&0xf0)==0xf0){ // 11110xxx
        len=4;
    }
    else if((c&0xe0)==0xe0){ // 1110xxxx
        len=3;
    }
    else if((c&0xc0)==0xc0){ // 11xxxxxx
        len=2;
    }
    else{
        throw InvalidFormatException("string should be uft-8 or ascii");
    }
    return len;
}

std::vector<std::string_view> AutoSuggestor::edge_ngram(const std::string_view query)
{
    std::vector<std::string_view> ngrams;
    size_t index=0;
    size_t i=0;
    for(;i<_min_gram&&index<query.length();i++){
        index+=get_character_len(query[index]);
    }
    while(index<=query.length()&&i<_max_gram){
        ngrams.push_back(query.substr(0,index));
        index+=get_character_len(query[index]);
        i++;
    }
    return ngrams;
}



} //namespace Search
} //namespace Skilo
