#include "CollectionManager.h"
namespace Skilo {

CollectionManager::CollectionManager(const std::string &db_path):_storage_service(std::make_unique<StorageService>(db_path))
{

}

std::optional<std::string> CollectionManager::create_collection(const std::string &collection_name,CollectionMeta &collection_meta)
{

    if(_collection_name_id_map.find(collection_name)!=_collection_name_id_map.end()||_storage_service->contain_collection(collection_name)){
        return std::make_optional<std::string>("The collection with name `"+collection_name+"` already exists");
    }
    try {
        uint32_t collection_id=this->get_next_collection_id();

        collection_meta.add_collection_id(collection_id);
        collection_meta.add_create_time(312312);//TODO
        if(!_storage_service->write_new_collection(_next_collection_id,collection_meta)){
            return std::make_optional<std::string>("Could not write meta data to on disk storage");
        }
        //TODO first record collection next seq id
        std::unique_ptr<Collection> new_colletion=std::make_unique<Collection>(collection_meta,_storage_service.get());
        _collection_name_id_map[collection_name]=collection_id;
        _collection_map[collection_id]=std::move(new_colletion);

    }  catch (const std::runtime_error &err) {
        return std::make_optional<std::string>(std::string("error when creating collection: ")+err.what());
    }
    return std::nullopt;
}

std::optional<std::string> Skilo::CollectionManager::add_document(uint32_t collection_name,const Document &document)
{

}

uint32_t CollectionManager::get_next_collection_id()
{
    return _next_collection_id++;
}

} //namespace Skilo
