#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "cppjieba/Jieba.hpp"

namespace Skilo {
namespace Index {

/// @class tokenize the text and apply linguistic processing(e.g remove stop words)
/// @threadsafe The Tokenizer interfaces must be safe to call from different threads
class TokenizeStrategy
{
public:
    TokenizeStrategy(const std::string &dict_dir={});
    virtual ~TokenizeStrategy()=default;

    /// @return word->offsets(offsets could be empty)
    virtual std::unordered_map<std::string, std::vector<uint32_t>> tokenize(const std::string &text)=0;
};

class DefaultTokenizer:public TokenizeStrategy
{
public:
    DefaultTokenizer();
    virtual ~DefaultTokenizer() override=default;

    /// @return word->offsets
    virtual std::unordered_map<std::string, std::vector<uint32_t>> tokenize(const std::string &text) override;
};

class JiebaTokenizer:public TokenizeStrategy{
public:
    JiebaTokenizer(const std::string &dict_dir);
    virtual ~JiebaTokenizer() override=default ;

    /// @return a set of "unicode word and unicode offsets"
    virtual std::unordered_map<std::string, std::vector<uint32_t>> tokenize(const std::string &text) override;

    /// @brief load stop words from file
    /// @return the num of stop words loaded
    size_t load_stop_words(const std::string &file_path);

private:
    const char* const DICT_PATH = "/jieba.dict.utf8";
    const char* const HMM_PATH = "/hmm_model.utf8";
    const char* const USER_DICT_PATH = "/user.dict.utf8";
    const char* const IDF_PATH = "/idf.utf8";
    const char* const STOP_WORD_PATH = "/stop_words.utf8";

    cppjieba::Jieba _jieba;
    std::unordered_set<std::string> _stop_words;
};

#endif // TOKENIZER_H

} //namespace Index
} //namespace Skilo
