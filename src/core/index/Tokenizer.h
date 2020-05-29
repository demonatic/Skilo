#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>
#include "cppjieba/Jieba.hpp"


namespace Skilo {
namespace Index {

using std::string;


struct TokenSet{
    TokenSet(std::unordered_map<string,std::vector<uint32_t>> &word_to_offests):_term_to_offsets(std::move(word_to_offests))
    {

    }

    bool empty() const;
    bool contain_token(const std::string &token) const;

    size_t size() const;

    std::vector<uint32_t> get_offsets(const std::string &term) const;
    const std::unordered_map<string, std::vector<uint32_t>> &term_to_offsets() const;

    const std::vector<string>& get_fuzzies(const std::string &term,const size_t distance,std::function<std::vector<std::vector<string>>(size_t max_distance)> fuzzy_term_loader) const;

    void drop_token(const std::string &term);

private:
    std::unordered_map<string,std::vector<uint32_t>> _term_to_offsets;

    struct fuzzy_item{
        //i bit is set to 1 means the fuzzy term with edit distance i has been calculated
        std::bitset<4> distance_calculated;
        //index: edit distance, element: fuzzy matches
        std::vector<std::vector<string>> fuzzy_terms;
    };
    mutable std::unordered_map<std::string,fuzzy_item> _fuzzy; //defer fetch until being used
};

/// @class tokenize the text and apply linguistic processing(e.g remove stop words)
/// @threadsafe The Tokenizer interfaces must be safe to call from different threads
class TokenizeStrategy
{
public:
    TokenizeStrategy(const std::string &dict_dir={});
    virtual ~TokenizeStrategy()=default;

    /// @return word->offsets(offsets could be empty)
    virtual TokenSet tokenize(const std::string &text) const=0;
};

class DefaultTokenizer:public TokenizeStrategy
{
public:
    DefaultTokenizer();
    virtual ~DefaultTokenizer() override=default;

    /// @return word->offsets
    virtual TokenSet tokenize(const std::string &text) const override;
};

class JiebaTokenizer:public TokenizeStrategy{
public:
    JiebaTokenizer(const std::string &dict_dir);
    virtual ~JiebaTokenizer() override=default ;

    /// @return a set of "unicode word and unicode offsets"
    virtual TokenSet tokenize(const std::string &text) const override;

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

} //namespace Index
} //namespace Skilo

#endif // TOKENIZER_H

