#include "Collection.h"
#include "g3log/g3log.hpp"
#include "search/IndexSearcher.h"

namespace Skilo {

using namespace Schema;

Collection::Collection(const CollectionMeta &collection_meta,StorageService *storage_service,const SkiloConfig &config):
    _collection_id(collection_meta.get_collection_id()),
    _collection_name(collection_meta.get_collection_name()),
    _storage_service(storage_service),_schema(collection_meta),
    _indexes(_collection_id,_schema,collection_meta,_storage_service),_config(config)
{
    _next_seq_id=_storage_service->get_collection_next_seq_id(_collection_id);
    _tokenizer=this->get_tokenize_strategy(collection_meta.get_tokenizer());

    this->build_index();
}

uint32_t Collection::get_id() const
{
    return this->_collection_id;
}

const std::string &Collection::get_name() const
{
    return this->_collection_name;
}

void Collection::build_index()
{
    size_t load_doc_count=0;
    LOG(INFO)<<"Loading documents of collection \""<<_collection_name<<"\" ...";
    _storage_service->scan_for_each_doc(_collection_id,[this,&load_doc_count](const std::string_view doc_str){
        Index::IndexWriter index_writer(_indexes,_tokenizer.get());
        const Document doc(doc_str);
        index_writer.index_in_memory(_schema,doc);
        load_doc_count++;
    });
    LOG(INFO)<<"Collection \""<<_collection_name<<"\" load finished. total "<<load_doc_count<<" documents";
    _doc_num=load_doc_count;
    _indexes.set_doc_num(_doc_num);
}

void Collection::drop_all()
{
    _storage_service->drop_collection(_collection_id,_collection_name);
}

void Collection::add_new_document(Document &document)
{
    this->validate_document(document);
    document.add_seq_id(_next_seq_id);
    if(!_storage_service->write_document(_collection_id,document)){
        std::string err="fail to write document to storage with doc id=\""+std::to_string(document.get_doc_id())+"\"";
        LOG(WARNING)<<err;
        throw InternalServerException(err);
    }
    _next_seq_id++;
    Index::IndexWriter index_writer(_indexes,_tokenizer.get());
    index_writer.index_in_memory(_schema,document);
    _indexes.set_doc_num(++_doc_num);
}

void Collection::remove_document(const uint32_t doc_id)
{
    uint32_t seq_id=_storage_service->get_doc_seq_id(_collection_id,doc_id);
    Document doc=this->_storage_service->get_document(_collection_id,seq_id);
    Index::IndexEraser index_eraser(_indexes,_tokenizer.get());
    index_eraser.remove_from_memory_index(_schema,doc);
    _indexes.set_doc_num(--_doc_num);
    if(!_storage_service->remove_document(_collection_id,doc)){
        std::string err="fail to remove document from storage with doc id=\""+std::to_string(doc.get_doc_id())+"\"";
        LOG(WARNING)<<err;
        throw InternalServerException(err);
    }
}

bool Collection::contain_document(const uint32_t doc_id) const
{
    return _storage_service->contain_document(_collection_id,doc_id);
}

void Collection::validate_document(const Document &document)
{
    return _schema.validate(document);
}

SearchResult Collection::search(const Query &query_info) const
{
    Search::IndexSearcher searcher(query_info,_indexes,_tokenizer.get());

    std::vector<std::pair<uint32_t,double>> res_docs;
    float search_time_cost=Util::timing_function(
        std::bind(&Search::IndexSearcher::search,searcher,std::placeholders::_1),res_docs);

    uint32_t hit_count=static_cast<uint32_t>(res_docs.size());

    //load hit documents
    SearchResult result(hit_count);
    for(auto [seq_id,score]:res_docs){
        LOG(DEBUG)<<"search hit seq_id="<<seq_id<<" score="<<score;
        Document doc=_storage_service->get_document(_collection_id,seq_id);
        result.add_hit(doc,score);
    }
    result.add_took_secs(search_time_cost);
    return result;
}

uint32_t Collection::get_doc_num() const
{
    return _doc_num;
}

std::vector<std::string_view> Collection::auto_suggest(const std::string &query_prefix) const
{
    Search::AutoSuggestor *suggestor=_indexes.get_suggestor();
    if(!suggestor){
        throw UnAuthorizedException("auto suggestion is not enabled in schema");
    }
    return suggestor->auto_suggest(query_prefix);
}

std::unique_ptr<Index::TokenizeStrategy> Collection::get_tokenize_strategy(const std::string &tokenizer_name) const
{
    using StrategyFactory=std::function<std::unique_ptr<Index::TokenizeStrategy>()>;
    static const std::unordered_map<std::string,StrategyFactory> factories{
        {"default",[](){return std::make_unique<Index::DefaultTokenizer>();}},
        {"jieba",[](){return std::make_unique<Index::JiebaTokenizer>(SkiloConfig::get_conf<std::string>("extensions.dict_dir"));}}
    };
    auto it=factories.find(tokenizer_name);
    return it!=factories.end()?it->second():this->get_tokenize_strategy("default");
}


} //namespace Skilo
