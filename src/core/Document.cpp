#include "Document.h"
#include <stdexcept>
#include "../../3rd/include/rapidjson/stringbuffer.h"
#include "../../3rd/include/rapidjson/writer.h"

Document::Document(uint32_t collection_id,uint32_t seq_id,const std::string &json_str):
    _seq_id(seq_id),_collection_id(collection_id)
{
    if(_document.Parse(json_str.c_str()).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json");
    }
    this->init();
}

Document::Document(uint32_t collection_id,uint32_t seq_id,const SegmentBuf &json_str):
    _seq_id(seq_id),_collection_id(collection_id)
{
    detail::SegmentBufferStream stream(json_str);
    if(_document.ParseStream(stream).HasParseError()||!_document.IsObject()){
        throw std::runtime_error("Error when parse document from json");
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
    const std::string doc_key(KeyConverter::doc_id_key(_collection_id,_doc_id));
    const std::string seq_key(KeyConverter::doc_seq_key(_collection_id,_seq_id));
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
