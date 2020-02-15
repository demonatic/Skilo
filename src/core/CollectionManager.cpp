#include "CollectionManager.h"
#include <g3log/g3log.hpp>
#include <future>

namespace Skilo {

CollectionManager::CollectionManager(const SkiloConfig &config)
    :_config(config),_storage_service(std::make_unique<StorageService>(config.get_db_dir()))
{
   this->init_collections();
}

void CollectionManager::init_collections()
{
    this->_next_collection_id=_storage_service->get_next_collection_id();
    std::cout<<"@CollectionManager::init_collection  collection id="<<_next_collection_id<<std::endl;
    std::vector<CollectionMeta> collection_meta=_storage_service->get_all_collection_meta();

    std::vector<std::future<std::unique_ptr<Collection>>> init_futures;
    for(CollectionMeta &meta:collection_meta){
        init_futures.emplace_back(std::async([this,&meta](){
            LOG(INFO)<<"Loading collection \""<<meta.get_collection_name()<<"\"";
            return std::make_unique<Collection>(meta,_storage_service.get(),_config);
        }));
    }

    for(size_t i=0;i<init_futures.size();i++){
        CollectionMeta &meta=collection_meta[i];
        uint32_t collection_id=meta.get_collection_id();
        const std::string collection_name=meta.get_collection_name();
        _collection_name_id_map[collection_name]=collection_id;
        _collection_map[collection_id]=init_futures[i].get();
    }
    LOG(INFO)<<"Loading all collections finished";
}

Status CollectionManager::create_collection(CollectionMeta &collection_meta)
{
    try {
        const std::string &collection_name=collection_meta.get_collection_name();
        LOG(INFO)<<"Attemping to create collection \""<<collection_name<<"\"";

        if(get_collection(collection_name)!=nullptr||_storage_service->contain_collection(collection_name)){
            return Status{RetCode::CONFLICT,"The collection with name `"+collection_name+"` already exists"};
        }

        uint32_t collection_id=this->get_next_collection_id();
        std::cout<<"@CollectionManager::create_collection  collection id="<<collection_id<<std::endl;
        collection_meta.add_collection_id(collection_id);
        collection_meta.add_create_time(static_cast<uint64_t>(std::time(nullptr)));
        if(!_storage_service->write_new_collection(collection_id,collection_meta)){
            throw InternalServerException("Could not write meta data to on disk storage");
        }

        std::unique_ptr<Collection> new_colletion=std::make_unique<Collection>(collection_meta,_storage_service.get(),_config);
        _collection_name_id_map[collection_name]=collection_id;
        _collection_map[collection_id]=std::move(new_colletion);
        LOG(INFO)<<"Collection \""<<collection_name<<"\" id="<<collection_id<<" has created";

    } catch (const InvalidFormatException &err){
        return Status{RetCode::BAD_REQUEST,err.what()};

    } catch(const InternalServerException &err){
        return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};

    }catch(const ConflictException &err){
        return Status{RetCode::CONFLICT,err.what()};

    }catch(const std::exception &err){
        return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};
    }
    return Status{RetCode::CREATED,"Create Collection OK"};
}

Status Skilo::CollectionManager::add_document(const std::string &collection_name,Document &document)
{
    try {
        Collection *collection=this->get_collection(collection_name);
        if(!collection){
            throw NotFoundException("collection \""+collection_name+"\" not exist");
        }
        uint32_t doc_id=document.get_doc_id();
        if(collection->contain_document(doc_id)){
            throw ConflictException("The collection with name `"+collection_name+"` has already contained doc "+std::to_string(doc_id));
        }
        collection->add_new_document(document);

    } catch (const InvalidFormatException &err){
        return Status{RetCode::BAD_REQUEST,err.what()};

    } catch(const InternalServerException &err){
        return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};

    }catch(const NotFoundException &err){
        return Status{RetCode::NOT_FOUND,err.what()};

    }catch(const ConflictException &err){
        return Status{RetCode::CONFLICT,err.what()};

    }catch(const std::exception &err){
        return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};
    }
    return Status{RetCode::CREATED,"Add Document Success"};
}

Status CollectionManager::add_document_batch(const string &collection_name, DocumentBatch &doc_batch)
{
    size_t add_count=0;
    Status batch_add_stat;
    string err_reason;
    std::vector<Document> &docs=doc_batch.get_docs();
    for(Document &doc:docs){
        Status add_doc_stat=this->add_document(collection_name,doc);
        if(add_doc_stat.code==RetCode::CREATED){
            add_count++;
        }
        else{
            if(!err_reason.empty()) err_reason.push_back(',');
            err_reason.append(std::move(add_doc_stat.description));
            batch_add_stat.code=add_doc_stat.code;
        }
    }
    if(add_count){
        batch_add_stat.code=RetCode::CREATED;
    }
    size_t fail_count=docs.size()-add_count;
    batch_add_stat.description=std::to_string(add_count)+" docs added, "+std::to_string(fail_count)+" failed"+(fail_count?" with reason: "+err_reason:"");
    return batch_add_stat;
}

Status CollectionManager::search(const Query &query_info) const
{
     std::string query_result;
     try {
         const std::string &collection_name=query_info.get_collection_name();
         Collection *collection=this->get_collection(collection_name);
         if(!collection){
             throw NotFoundException("collection \'"+collection_name+"\" not exist");
         }
         SearchResult result=collection->search(query_info);
         query_result=result.dump();
     }
     catch(const NotFoundException &err){
         return Status{RetCode::NOT_FOUND,err.what()};
     }
     catch(const InternalServerException &err){
         return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};
     }
     catch(const std::exception &err){
         return Status{RetCode::INTERNAL_SERVER_ERROR,err.what()};
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
