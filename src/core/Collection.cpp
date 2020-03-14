#include "Collection.h"
#include "g3log/g3log.hpp"

namespace Skilo {

using namespace Schema;

Collection::Collection(const CollectionMeta &collection_meta,StorageService *storage_service,const SkiloConfig &config):
    _storage_service(storage_service),_schema(collection_meta),
    _indexes(_collection_id,_schema,_storage_service),_config(config)
{
    _collection_id=collection_meta.get_collection_id();
    _collection_name=collection_meta.get_collection_name();
    _next_seq_id=_storage_service->get_collection_next_seq_id(_collection_id);
    _indexes.set_doc_num(_next_seq_id);
    _tokenizer=this->get_tokenize_strategy(collection_meta.get_tokenizer());
    cout<<"@Collection constructor next_seq_id="<<_next_seq_id<<endl;
    this->build_index();
}

void Collection::build_index()
{
    size_t load_doc_count=0;
    _storage_service->scan_for_each_doc(_collection_id,[this,&load_doc_count](const std::string_view doc_str){
        Index::IndexWriter index_writer(_indexes,_tokenizer.get());
        const Document doc(doc_str);
        index_writer.index_in_memory(_schema,doc);
        load_doc_count++;
    });
    LOG(DEBUG)<<"Collection \""<<_collection_name<<"\" load finished. total "<<load_doc_count<<" documents";
}

void Collection::add_new_document(Document &document)
{
    this->validate_document(document);
    document.add_seq_id(_next_seq_id);
    if(!_storage_service->write_document(_collection_id,document)){
        std::string err="fail to write document with doc_id="+std::to_string(document.get_doc_id());
        LOG(WARNING)<<err;
        throw InternalServerException(err);
    }
    _next_seq_id++;
    Index::IndexWriter index_writer(_indexes,_tokenizer.get());
    index_writer.index_in_memory(_schema,document);
    _indexes.set_doc_num(_next_seq_id);
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
    //extract and split query terms and fields
    const std::string &query_str=query_info.get_search_str();
    std::unordered_map<std::string, std::vector<uint32_t>> query_terms=_tokenizer->tokenize(query_str);
    const vector<std::string> &search_fields=query_info.get_query_fields();

    //init search criteria and do search
    uint32_t top_k=50;
    Search::HitCollector collector(top_k,this->get_scorer("tfidf"));
    this->_indexes.search_fields(query_terms,search_fields,collector);

    //collect hit documents
    std::vector<pair<uint32_t,float>> res_docs=collector.get_top_k();
    uint32_t hit_count=static_cast<uint32_t>(res_docs.size());

    //load hit documents
    SearchResult result(hit_count);
    for(auto [seq_id,score]:res_docs){
        std::cout<<"@collection search result: seq_id="<<seq_id<<" score="<<score<<endl;
        Document doc=_storage_service->get_document(_collection_id,seq_id);
        result.add_hit(doc,score);
    }
    return result;
}

uint32_t Collection::document_num() const
{
    return _next_seq_id;
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

std::unique_ptr<Search::Scorer> Collection::get_scorer(const string &scorer_name) const
{
    using ScorerFactory=std::function<std::unique_ptr<Search::Scorer>()>;
    static const std::unordered_map<std::string,ScorerFactory> factories{
        {"default",[](){return std::make_unique<Search::TFIDF_Scorer>();}},
        {"tfidf",[](){return std::make_unique<Search::TFIDF_Scorer>();}},
    };
    auto it=factories.find(scorer_name);
    return it!=factories.end()?it->second():this->get_scorer("default");
}


} //namespace Skilo
