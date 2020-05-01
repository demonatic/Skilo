#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "utility/Exception.h"
#include <vector>
#include <optional>

namespace Skilo {

using SegmentBuf=std::vector<std::pair<uint8_t*,size_t>>;

class DocumentBase
{
public:
    DocumentBase();
    /// @throw InvalidFormatException when parse json failed
    DocumentBase(const std::string_view json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    DocumentBase(const SegmentBuf &json_str);

    bool contain_key(const std::string &key) const;

    std::string dump() const;

    rapidjson::Document &get_raw();
    const rapidjson::Document& get_raw() const;

    std::optional<uint32_t> extract_opt_int(const rapidjson::Value &v,const std::string &field_name) const;

protected:
    rapidjson::Document _document;
};

/// Collection Document format example:
/// "id" field is required in collection document but could omit in the schema
/****************************************************
{
    "id": 1001,
    "product name":{
        "type":"string",
        "index":true,
        "suggest":true
    },
    "price":{
        "type":"float"
    }
}
***************************************************/

class Document:public DocumentBase{
public:
    Document(DocumentBase &base);
    Document(rapidjson::Value doc, decltype(_document.GetAllocator()) &allocator);

    Document(const std::string_view json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    Document(const SegmentBuf &json_str);

    uint32_t get_doc_id() const;
    const std::string& get_collection_name() const;
    void add_seq_id(uint32_t seq_id);
    std::optional<uint32_t> get_seq_id() const;

    /// @path for array: aaa.<element_index>.ccc
    rapidjson::Value &get_value(const std::string &field_path);

private:
    void extract_variables();

private:
    uint32_t _doc_id;
    std::optional<uint32_t> _seq_id;
};

/// DocumentBatch format example:
/********************************
{
    "docs":[
        <doc1>,
        <doc2>,
        <doc3>
    ]
}
********************************/
class DocumentBatch:public DocumentBase{
public:
    DocumentBatch(DocumentBase &base);
    DocumentBatch(const std::string &collection_name,const std::string_view json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    DocumentBatch(const std::string &collection_name,const SegmentBuf &json_str);

    std::vector<Document>& get_docs();
private:
    void extract_variables();

private:
    std::string _collection_name;
    std::vector<Document> _docs;
};

/// ColletionMeta data example:
/// where "id" and "create time" are added by Skilo automatically
/****************************************************
{
    "name": "company products",
    "tokenizer":"jieba",
    "schema": {
        "type":"object",
        "$fields": {
            "product name":{
                "type":"string",
                "index":true
            },
            "price":{
                "type":"float"
            }
        }
    },
    "auto_suggestion":{
        "entry_num":5,
        "min_gram":2,
        "max_gram":15
    },
    "id":1,
    "create time":5342534432
}
***************************************************/


class CollectionMeta:public DocumentBase{
public:
    CollectionMeta(const std::string_view json_str);
    CollectionMeta(const SegmentBuf &json_str);

    uint32_t get_collection_id() const;
    const std::string& get_collection_name() const;

    /// @throw runtime error if "schema" field is not found
    const rapidjson::Value &get_schema() const;

    const std::string& get_tokenizer() const;

    bool enable_auto_suggestion() const;

    void add_create_time(uint64_t created_time);
    void add_collection_id(uint32_t collection_id);

    uint32_t get_min_gram(const uint32_t default_min_gram) const;
    uint32_t get_max_gram(const uint32_t default_max_gram) const;
    uint32_t get_suggest_entry_num(const uint32_t default_suggest_entry_num) const;

private:
    void extract_variables();

private:
    bool _enable_auto_suggestion;
    std::optional<uint32_t> _min_gram,_max_gram,_suggest_entry_num;

    std::optional<uint32_t> _collection_id;
    std::string _collection_name;
    std::string _tokenizer_name;

};

/// SearchInfo example:
/// @note "query by" format is xxx.xxx... if sechema is nested
/****************************************************
{
    "query": "iphone",
    "query by": ["product name","price"],
    "sort by":["price:asc","sales:desc"]
}
***************************************************/

class Query:public DocumentBase{
public:
    using SortInAscend=bool;

    Query(const std::string &collection_name,const std::string_view json_str);
    Query(const std::string &collection_name,const SegmentBuf &json_str);

    const std::string &get_collection_name() const;
    const std::string &get_search_str() const;

    const std::vector<std::string>& get_query_fields() const;
    const std::vector<std::pair<std::string,SortInAscend>>& get_sort_fields() const;

private:
    void extract_variables();

private:
    std::string _collection_name;
    std::string _search_str;
    std::vector<std::string> _query_fields;
    std::vector<std::pair<std::string,SortInAscend>> _sort_fields;
};

///SearchResult example:
/******************************************************
{
  "found": 1,
  "took ms": 1,
  "hits": [
    {
        "id": "124",
        "product name": "iphoneXS MAX PLUS PRO",
        "price": 16432.5
    }
  ],
  "scores":[4.12]
}

******************************************************/
class SearchResult:public DocumentBase{
public:
    SearchResult(uint32_t num_found);
    void add_hit(Document &doc,double score);
    void add_took_ms(float ms);
};

///AutoSuggestion result example with query prefix="I love":
/******************************************************
{
  "suggestions": [
    "I love C++",
    "I love Python",
    "I love swimming in the ocean"
  ]
}
******************************************************/

class AutoSuggestion:public DocumentBase{
public:
    AutoSuggestion();
    void add_suggestion(std::string_view suggestion);
};

namespace detail{

struct SegmentBufferStream{
    using Ch=uint8_t;
    SegmentBufferStream(const SegmentBuf &buffer_segment):
        _buffer_segment(buffer_segment),_it(_buffer_segment.begin()),_cur(_it!=_buffer_segment.end()?_it->first:nullptr),_seg_end(_cur+_it->second),_count(0)
    {

    }
    //! 从流读取当前字符，不移动读取指针（read cursor）
    Ch Peek() const {
        return _cur!=nullptr?*_cur:'\0';
    }
    //! 从流读取当前字符，移动读取指针至下一字符。
    Ch Take() {
        if(!_cur) return '\0';
        Ch c=*_cur;
        _cur++;
        if(_cur==_seg_end){
            ++_it;
            if(_it==_buffer_segment.cend()){
                _cur=nullptr;
            }
            else{
                _cur=_it->first;
                _seg_end=_cur+_it->second;
            }
        }
        _count++;
        return c;
    }
    /** 获取读取指针。
    *  \return 从开始以来所读过的字符数量。 */
    size_t Tell() const {
        return _count;
    }

    [[ noreturn ]] Ch* PutBegin() {assert(false);}
    [[ noreturn ]] void Put(Ch) { assert(false);}
    [[ noreturn ]] void Flush() {assert(false);}
    [[ noreturn ]] size_t PutEnd(Ch*) { assert(false);}
    [[ noreturn ]] const Ch* Peek4() const {assert(false);}

    const std::vector<std::pair<Ch *,size_t>> &_buffer_segment;
    SegmentBuf::const_iterator _it;
    Ch *_cur;
    Ch *_seg_end;
    size_t _count;
};

}
} //namespace Skilo

#endif // DOCUMENT_H
