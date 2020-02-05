#include "Tokenizer.h"
#include <fstream>
#include <regex>
#include <g3log/g3log.hpp>

namespace Skilo {
namespace Index {

TokenizeStrategy::TokenizeStrategy(const std::string &dict_dir)
{

}

JiebaTokenizer::JiebaTokenizer(const std::string &dict_dir):TokenizeStrategy(dict_dir),
    _jieba(dict_dir+DICT_PATH,dict_dir+HMM_PATH,dict_dir+USER_DICT_PATH,dict_dir+IDF_PATH,dict_dir+STOP_WORD_PATH)
{
    this->load_stop_words(dict_dir+STOP_WORD_PATH);
}

std::unordered_map<std::string, std::vector<uint32_t>> JiebaTokenizer::tokenize(const std::string &text)
{
    std::vector<cppjieba::Word> words;
    _jieba.CutForSearch(text,words,true);
    std::unordered_map<std::string, std::vector<uint32_t>> res;
    for(cppjieba::Word &w:words){
        if(!_stop_words.count(w.word)){
            res[std::move(w.word)].push_back(w.unicode_offset);
        }
    }
    return res;
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

std::unordered_map<std::string, std::vector<uint32_t> > DefaultTokenizer::tokenize(const std::string &text)
{
    std::regex reg("[^\\ \\.|,:;&]+");
    std::sregex_iterator first{text.begin(),text.end(),reg},last;

    std::unordered_map<std::string, std::vector<uint32_t>> res;
    while(first!=last){
        uint32_t offset=static_cast<uint32_t>(first->position());
        res[first->str()].push_back(offset);
        first++;
    }
    return res;
}

} //namespace Index
} //namespace Skilo
