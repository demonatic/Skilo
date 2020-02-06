#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include <vector>
#include <optional>

namespace Skilo {

using SegmentBuf=std::vector<std::pair<uint8_t*,size_t>>;

class DocumentBase
{
public:
    DocumentBase();
    /// @throw std::runtime_error when parse json failed
    DocumentBase(const std::string &json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    DocumentBase(const SegmentBuf &json_str);

    std::string dump() const;

    const rapidjson::Document& get_raw() const;
protected:
    rapidjson::Document _document;
};

/// Collection Document example:
/// "id" field is required in collection document but could omit in the schema
/****************************************************
{
    "id": 1001,
    "product name":{
        "type":"string"
        "index":true
    },
    "price":{
        "type":"float"
    }
}
***************************************************/

class Document:public DocumentBase{
public:
    Document(const std::string &collection_name,const std::string &json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    Document(const std::string &collection_name,const SegmentBuf &json_str);

    uint32_t get_doc_id() const;
    const std::string& get_collection_name() const;
    void set_seq_id(uint32_t seq_id);
    std::optional<uint32_t> get_seq_id() const;

private:
    uint32_t _doc_id;
    std::optional<uint32_t> _seq_id;
    std::string _collection_name;
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
    }
    "id":1
    "create time":5342534432
}
***************************************************/


class CollectionMeta:public DocumentBase{
public:
    CollectionMeta(const std::string &json_str);
    CollectionMeta(const SegmentBuf &json_str);

    /// @throw runtime error if "schema" field is not found
    const rapidjson::Value &get_schema() const;

    const std::string& get_collection_name() const;
    const std::string& get_tokenizer() const;

    void add_create_time(uint64_t created_time);
    void add_collection_id(uint32_t collection_id);

private:
    void init();

private:
    std::string _collection_name;
    std::string _tokenizer_name;
};

/// SearchInfo example:
/// @note "query by" format is xxx.xxx... if sechema is nested
/****************************************************
{
    "query": "iphone",
    "query by": ["product name","price"]
}
***************************************************/

class Query:public DocumentBase{
public:
    Query(const std::string &collection_name,const std::string &json_str);
    Query(const std::string &collection_name,const SegmentBuf &json_str);

    const std::string &get_collection_name() const;
    const std::string &get_search_str() const;
    const std::vector<std::string>& get_query_fields() const;

private:
    void extract_variables();
private:
    std::string _collection_name;
    std::string _search_str;
    std::vector<std::string> _query_fields;
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
  ]
}

******************************************************/
class SearchResult:public DocumentBase{
public:
    SearchResult(uint32_t num_found);
    void add_hit(Document &doc);
    void add_took_ms(float ms);
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
