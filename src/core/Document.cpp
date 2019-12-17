#include "Document.h"
#include <stdexcept>
#include "../../3rd/include/rapidjson/stringbuffer.h"
#include "../../3rd/include/rapidjson/writer.h"

Document::Document(const std::string &json_str,uint32_t seq_id):_seq_id(seq_id)
{
    if(_document.Parse(json_str.c_str()).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json_str: "+json_str);
    }
    this->init();
}

Document::Document(const SegmentBuf &json_str,uint32_t seq_id):_seq_id(seq_id)
{
    detail::SegmentBufferStream stream(json_str);
    if(_document.ParseStream(stream).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json_seg_buf");
    }
    this->init();
}

uint32_t Document::doc_id() const
{
    return _doc_id;
}

void Document::dump() const
{

}

bool Document::write_to_storage(Storage *storage)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    _document.Accept(writer);
    Storage::Batch batch;
    //TODO
    std::string doc_key=std::to_string(_doc_id);
    std::string seq_key=std::to_string(_seq_id);
    batch.put(doc_key,seq_key);
    batch.put(seq_key,buffer.GetString());
    return storage->batch_write(batch);
}

void Document::init()
{
    const rapidjson::Value &doc_id=_document["id"];
    if(!doc_id.IsUint()){
        _doc_id=doc_id.GetUint();
    }
}
