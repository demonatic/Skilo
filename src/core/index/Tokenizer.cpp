#include "Tokenizer.h"
#include <fstream>
#include <regex>
#include <g3log/g3log.hpp>
#include "core/search/IndexSearcher.h"

namespace Skilo {
namespace Index {

bool TokenSet::empty() const
{
    return _term_to_offsets.empty();
}

bool TokenSet::contain_token(const std::string &token) const
{
    return _term_to_offsets.count(token);
}

size_t TokenSet::size() const
{
    return _term_to_offsets.size();
}

std::vector<uint32_t> TokenSet::get_offsets(const std::string &term) const
{
    auto offset_it=_term_to_offsets.find(term);
    return offset_it!=_term_to_offsets.end()?offset_it->second:std::vector<uint32_t>{};
}

const std::unordered_map<std::string, std::vector<uint32_t> > &TokenSet::term_to_offsets() const
{
     return _term_to_offsets;
}

const std::vector<string> &TokenSet::get_fuzzies(const std::string &term,const size_t distance,std::function<std::vector<std::vector<string>>(const size_t max_distance)> fuzzy_term_loader) const
{
    assert(distance<4);;
    if(!_fuzzy.count(term)){
        _fuzzy.emplace(term,fuzzy_item{std::bitset<4>("0000"),{}});
    }
    fuzzy_item &item=_fuzzy[term];
    if(item.distance_calculated.test(distance)){
        return item.fuzzy_terms[distance];
    }

    item.fuzzy_terms=fuzzy_term_loader(distance);

    for(size_t i=0;i<=distance;i++){
        item.distance_calculated.set(i);
    }
    return item.fuzzy_terms[distance];
}

void TokenSet::drop_token(const std::string &term)
{
    _term_to_offsets.erase(term);
    _fuzzy.erase(term);
}


TokenizeStrategy::TokenizeStrategy([[maybe_unused]]const std::string &dict_dir)
{

}

JiebaTokenizer::JiebaTokenizer(const std::string &dict_dir):TokenizeStrategy(dict_dir),
    _jieba(dict_dir+DICT_PATH,dict_dir+HMM_PATH,dict_dir+USER_DICT_PATH,dict_dir+IDF_PATH,dict_dir+STOP_WORD_PATH)
{
    this->load_stop_words(dict_dir+STOP_WORD_PATH);
}

TokenSet JiebaTokenizer::tokenize(const std::string &text) const
{
    std::vector<cppjieba::Word> words;
    _jieba.CutForSearch(text,words,true);
    std::unordered_map<std::string, std::vector<uint32_t>> word_to_offsets;
    for(cppjieba::Word &w:words){
        if(!_stop_words.count(w.word)){
            word_to_offsets[std::move(w.word)].push_back(w.unicode_offset);
        }
    }
    return TokenSet(word_to_offsets);
}

size_t JiebaTokenizer::load_stop_words(const std::string &file_path)
{
    std::ifstream fstream(file_path);
    if(!fstream.is_open()){
        LOG(WARNING)<<"load stop words from file \""<<file_path<<"\" failed";
        return 0;
    }
    std::string line;
    while(std::getline(fstream,line)){
        _stop_words.emplace(std::move(line));
    }
    return _stop_words.size();
}

DefaultTokenizer::DefaultTokenizer():TokenizeStrategy()
{

}

TokenSet DefaultTokenizer::tokenize(const std::string &text) const
{
    std::regex reg("[^\\ \\.|,:;&]+");
    std::sregex_iterator first{text.begin(),text.end(),reg},last;

    std::unordered_map<std::string, std::vector<uint32_t>> word_to_offsets;
    while(first!=last){
        uint32_t offset=static_cast<uint32_t>(first->position());
        word_to_offsets[first->str()].push_back(offset);
        first++;
    }
    return TokenSet(word_to_offsets);
}

} //namespace Index
} //namespace Skilo
