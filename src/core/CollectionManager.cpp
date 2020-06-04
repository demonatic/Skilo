#include "CollectionManager.h"
#include <g3log/g3log.hpp>
#include <future>

namespace Skilo {

CollectionManager::CollectionManager(const SkiloConfig &config)
    :_config(config),_storage_service(std::make_unique<StorageService>(config.get_db_dir()))
{

}

void CollectionManager::init_collections()
{
    this->_next_collection_id=_storage_service->get_next_collection_id();
    std::vector<CollectionMeta> collection_meta=_storage_service->get_all_collection_meta();

    std::vector<std::future<std::shared_ptr<Collection>>> init_futures;
    for(CollectionMeta &meta:collection_meta){
        init_futures.emplace_back(std::async(std::launch::async,[this,&meta](){
            LOG(INFO)<<"Loading collection \""<<meta.get_collection_name()<<"\"";
            return std::make_shared<Collection>(meta,_storage_service.get(),_config);
        }));
    }

    for(size_t i=0;i<init_futures.size();i++){
        CollectionMeta &meta=collection_meta[i];
        uint32_t collection_id=meta.get_collection_id();
        const std::string collection_name=meta.get_collection_name();
        _collection_name_id_map.insert(collection_name,collection_id);
        _collection_map.insert(collection_id,init_futures[i].get());
    }
    LOG(INFO)<<"Loading all collections finished";
}

ResultStr CollectionManager::create_collection(CollectionMeta &collection_meta)
{
    const std::string &collection_name=collection_meta.get_collection_name();
    LOG(INFO)<<"Attemping to create collection \""<<collection_name<<"\"";

    if(get_collection(collection_name)!=nullptr||_storage_service->contain_collection(collection_name)){
        throw ConflictException("The collection with name `"+collection_name+"` already exists");
    }

    uint32_t collection_id=this->get_next_collection_id();
    collection_meta.add_collection_id(collection_id);
    collection_meta.add_create_time(static_cast<uint64_t>(std::time(nullptr)));
    if(!_storage_service->write_new_collection(collection_id,collection_meta)){
        throw InternalServerException("Could not write meta data to on disk storage");
    }

    std::unique_ptr<Collection> new_colletion=std::make_unique<Collection>(collection_meta,_storage_service.get(),_config);
    _collection_map.insert(collection_id,std::move(new_colletion));
    _collection_name_id_map.insert(collection_name,collection_id);
    LOG(INFO)<<"Collection \""<<collection_name<<"\" id="<<collection_id<<" has created";

    return "create collection with name \'"+collection_name+"\' ok";
}

ResultStr CollectionManager::add_document(const std::string &collection_name,Document &document)
{
    std::shared_ptr<Collection> collection=this->get_collection(collection_name);
    if(!collection){
        throw NotFoundException("collection \""+collection_name+"\" not exist");
    }
    uint32_t doc_id=document.get_doc_id();
    if(collection->contain_document(doc_id)){
        throw ConflictException("The collection with name `"+collection_name+"` has already contained doc "+std::to_string(doc_id));
    }
    collection->add_new_document(document);
    return "add document "+std::to_string(doc_id)+" success";
}

ResultStr CollectionManager::add_document_batch(const string &collection_name, DocumentBatch &doc_batch)
{
    string err_reason;
    std::vector<Document> &docs=doc_batch.get_docs();
    //TODO rollback when failed?
    for(Document &doc:docs){
        this->add_document(collection_name,doc);
    }
    return std::to_string(docs.size())+" docs added";
}

ResultStr CollectionManager::drop_collection(const std::string &collection_name)
{
    LOG(INFO)<<"Attemping to drop collection \""<<collection_name<<"\"";
    std::shared_ptr<Collection> target_collection=get_collection(collection_name);
    if(!target_collection){
        throw NotFoundException("The collection with name `"+collection_name+"` not exists");
    }
    _collection_name_id_map.erase(collection_name);
    target_collection->drop_all();
    _collection_map.erase(target_collection->get_id());
    LOG(INFO)<<"collection \""<<collection_name<<"\" has been dropped";
    return "drop collection \""+collection_name+"\" success";
}

ResultStr CollectionManager::search(const Query &query_info) const
{
     const std::string &collection_name=query_info.get_collection_name();
     std::shared_ptr<Collection> collection=get_collection(collection_name);
     if(!collection){
         throw NotFoundException("collection \'"+collection_name+"\" not exist");
     }
     SearchResult result=collection->search(query_info);
     return result.dump();
}

ResultStr CollectionManager::auto_suggest(const string &collection_name, const string &query_prefix) const
{
    std::shared_ptr<Collection> collection=get_collection(collection_name);
    if(!collection){
        throw NotFoundException("collection \'"+collection_name+"\" not exist");
    }
    AutoSuggestion auto_sugg;
    std::vector<std::string_view> suggs=collection->auto_suggest(query_prefix);
    for(auto &&sug:suggs){
        auto_sugg.add_suggestion(sug);
    }
    return auto_sugg.dump();
}

std::shared_ptr<Collection> CollectionManager::get_collection(const string &collection_name) const
{
    uint32_t cid;
    if(!_collection_name_id_map.find(collection_name,cid)){
        return nullptr;
    }

    std::shared_ptr<Collection> coll;
    _collection_map.find(cid,coll);
    return coll;
}

uint32_t CollectionManager::get_next_collection_id()
{
    return _next_collection_id++;
}

} //namespace Skilo
