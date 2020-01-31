#include "Tokenizer.h"
#include <fstream>
#include <g3log/g3log.hpp>

namespace Skilo {
namespace Index {

TokenizeStrategy::TokenizeStrategy()
{

}

JiebaTokenizer::JiebaTokenizer():_jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
{
    this->load_stop_words(STOP_WORD_PATH);
}

std::unordered_map<std::string, std::vector<uint32_t>> JiebaTokenizer::tokenize(const std::string &sentence)
{
    std::vector<Word> words;
    _jieba.CutForSearch(sentence,words,true);
    std::unordered_map<std::string, std::vector<uint32_t>> res;
    for(Word &w:words){
        if(!_stop_words.count(w.word)){
            res[std::move(w.word)].push_back(w.offset);
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

} //namespace Index
} //namespace Skilo
