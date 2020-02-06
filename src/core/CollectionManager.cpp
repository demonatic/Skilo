#include "CollectionManager.h"
namespace Skilo {

CollectionManager::CollectionManager(const std::string &db_path):_storage_service(std::make_unique<StorageService>(db_path))
{

}

Status CollectionManager::create_collection(const std::string &collection_name,CollectionMeta &collection_meta)
{
    if(get_collection(collection_name)!=nullptr||_storage_service->contain_collection(collection_name)){
        return Status{RetCode::CONFLICT,"The collection with name `"+collection_name+"` already exists"};
    }
    try {
        uint32_t collection_id=this->get_next_collection_id();
        collection_meta.add_collection_id(collection_id);
        collection_meta.add_create_time(static_cast<uint64_t>(std::time(nullptr)));
        if(!_storage_service->write_new_collection(_next_collection_id,collection_meta)){
            return Status{RetCode::INTERNAL_SERVER_ERROR,"Could not write meta data to on disk storage"};
        }

        std::unique_ptr<Collection> new_colletion=std::make_unique<Collection>(collection_meta,_storage_service.get());
        _collection_name_id_map[collection_name]=collection_id;
        _collection_map[collection_id]=std::move(new_colletion);

    }  catch (const std::runtime_error &err){
        return Status{RetCode::BAD_REQUEST,std::string("error when creating collection: ")+err.what()};
    }
    return Status{RetCode::CREATED,"create collection ok"};
}

Status Skilo::CollectionManager::add_document(const std::string &collection_name,Document &document)
{
    Collection *collection=this->get_collection(collection_name);
    if(!collection){
         return Status{RetCode::BAD_REQUEST,"collection \'"+collection_name+"\" not exist"};
    }
    //TODO use try catch?
    std::optional<std::string> err=collection->add_document(document);
    return err.has_value()?Status{RetCode::BAD_REQUEST,err.value()}:Status{RetCode::CREATED,"add document success"};
}

Status CollectionManager::search(const Query &query_info) const
{
     const std::string &collection_name=query_info.get_collection_name();
     Collection *collection=this->get_collection(collection_name);
     if(!collection){
          return Status{RetCode::BAD_REQUEST,"collection \'"+collection_name+"\" not exist"};
     }
     std::string query_result;
     try {
         SearchResult result=collection->search(query_info);
         query_result=result.dump();
     }  catch (std::runtime_error &err) {
        //TODO
         return Status{RetCode::BAD_REQUEST,err.what()};
     }
     return Status{RetCode::OK,std::move(query_result)};
}

Collection *CollectionManager::get_collection(const string &collection_name) const
{
    auto collection_id_it=_collection_name_id_map.find(collection_name);
    if(collection_id_it==_collection_name_id_map.end()){
        return nullptr;
    }
    uint32_t collection_id=collection_id_it->second;
    auto collection_it=_collection_map.find(collection_id);
    if(collection_it==_collection_map.end()){
        return nullptr;
    }
    Collection *collection=collection_it->second.get();
    return collection;
}

uint32_t CollectionManager::get_next_collection_id()
{
    return _next_collection_id++;
}

} //namespace Skilo
