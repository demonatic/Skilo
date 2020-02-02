#include "Collection.h"
#include "g3log/g3log.hpp"

namespace Skilo {

using namespace Schema;

Collection::Collection(const CollectionMeta &collection_meta,StorageService *storage_service):
    _schema(collection_meta),_indexes(_schema),_storage_service(storage_service)
{
    _collection_name=collection_meta.get_collection_name();
    std::optional<uint32_t> next_seq_id=_storage_service->get_collection_next_seq_id(_collection_name);
    if(!next_seq_id){
        throw std::runtime_error("error when get collection \""+_collection_name+"\" next seq id ");
    }
    _next_seq_id=next_seq_id.value();
    _tokenizer=this->get_tokenize_strategy(collection_meta.get_tokenizer());
}

std::optional<std::string> Collection::add_document(Document &document)
{
    std::optional<std::string> validation_err=this->validate_document(document);
    if(validation_err.has_value()){
        return validation_err;
    }
    document.set_seq_id(_next_seq_id);
    if(!_storage_service->write_document(_collection_id,document)){
        std::string err="fail to write document with doc_id="+std::to_string(document.get_doc_id());
        LOG(WARNING)<<err;
        return std::make_optional<std::string>(err);
    }
    _next_seq_id++;
    Index::IndexWriter index_writer(_indexes,_tokenizer.get());
    index_writer.index_in_memory(_schema,document);
    return std::nullopt;
}

std::optional<std::string> Collection::validate_document(const Document &document)
{
    return _schema.validate(document);
}

uint32_t Collection::document_num() const
{
    return _next_seq_id;
}

std::unique_ptr<Index::TokenizeStrategy> Collection::get_tokenize_strategy(const std::string &tokenizer_name) const
{
    using strategy_factory=std::function<std::unique_ptr<Index::TokenizeStrategy>()>;
    static const std::unordered_map<std::string,strategy_factory> factories{
        {"default",[](){return std::make_unique<Index::DefaultTokenizer>();}},
        {"jieba",[](){return std::make_unique<Index::JiebaTokenizer>("/home/demonatic/Projects/Engineering Practice/Skilo/3rd/dict/");}}
    };
    auto it=factories.find(tokenizer_name);
    return it!=factories.end()?it->second():get_tokenize_strategy("default");
}


} //namespace Skilo
