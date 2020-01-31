#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "cppjieba/Jieba.hpp"

namespace Skilo {
namespace Index {

class TokenizeStrategy
{
public:
    TokenizeStrategy();
    virtual ~TokenizeStrategy()=default;

    /// @return word->offsets
    virtual std::unordered_map<std::string, std::vector<uint32_t>> tokenize(const std::string &sentence)=0;
};

/// @threadsafe The JiebaTokenizer is threadsafe
class JiebaTokenizer{
    using Word=cppjieba::Word;
public:
    JiebaTokenizer();
    virtual ~JiebaTokenizer()=default;
    virtual std::unordered_map<std::string, std::vector<uint32_t>> tokenize(const std::string &sentence);

    /// @brief load stop words from file
    /// @return the num of stop words loaded
    size_t load_stop_words(const std::string &file_path);

private:
    const char* const DICT_PATH = "../dict/jieba.dict.utf8";
    const char* const HMM_PATH = "../dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "../dict/user.dict.utf8";
    const char* const IDF_PATH = "../dict/idf.utf8";
    const char* const STOP_WORD_PATH = "../dict/stop_words.utf8";

    cppjieba::Jieba _jieba;
    std::unordered_set<std::string> _stop_words;
};

#endif // TOKENIZER_H

} //namespace Index
} //namespace Skilo
