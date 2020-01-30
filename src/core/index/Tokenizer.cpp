#include "Tokenizer.h"
#include <fstream>

namespace Skilo {
namespace Index {

TokenizeStrategy::TokenizeStrategy()
{

}

JiebaTokenizer::JiebaTokenizer():_jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH)
{

}

std::vector<std::string> JiebaTokenizer::tokenize(const std::string &sentence)
{
    std::vector<std::string> words;
    _jieba.CutForSearch(sentence,words,true);
    return words;
}

size_t JiebaTokenizer::load_stop_words(const std::string &file_path)
{
    std::ifstream fstream(file_path);
    if(!fstream.is_open()){
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
