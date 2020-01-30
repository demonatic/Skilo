#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <set>
#include "cppjieba/Jieba.hpp"

namespace Skilo {
namespace Index {

class TokenizeStrategy
{
public:
    TokenizeStrategy();
    virtual ~TokenizeStrategy()=default;
    virtual std::vector<std::string> tokenize(const std::string &sentence)=0;
};


class JiebaTokenizer{
    using Word=cppjieba::Word;
public:
    JiebaTokenizer();
    virtual ~JiebaTokenizer()=default;
    virtual std::vector<std::string> tokenize(const std::string &sentence);

private:
    const char* const DICT_PATH = "../dict/jieba.dict.utf8";
    const char* const HMM_PATH = "../dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "../dict/user.dict.utf8";
    const char* const IDF_PATH = "../dict/idf.utf8";
    const char* const STOP_WORD_PATH = "../dict/stop_words.utf8";

    cppjieba::Jieba _jieba;
    std::set<std::string> _stop_words;
};

#endif // TOKENIZER_H

} //namespace Index
} //namespace Skilo
