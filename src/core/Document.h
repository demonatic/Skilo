#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <../../3rd/include/rapidjson/rapidjson.h>
#include <../../3rd/include/rapidjson/document.h>
#include "../../3rd/include/rapidjson/memorybuffer.h"
#include <vector>

using SegmentBuf=std::vector<std::pair<uint8_t *,size_t>>;

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

class Document
{
public:
    Document(const std::string &json_str);
    /// @brief parse json from some segment of buffers,
    /// where uint8_t* points to start of this segment and size_t indicates the length of this segment
    Document(const SegmentBuf &json_seg_buf);

    std::string doc_id() const;
    void dump() const;

private:
    std::string _doc_id;
    rapidjson::Document _document;

};

#endif // DOCUMENT_H