#include "CollectionManager.h"
namespace Skilo {

CollectionManager::CollectionManager(const std::string &db_path):_storage(std::make_unique<Storage>(db_path))
{

}

std::optional<std::string> CollectionManager::create_collection(const std::string &collection_name,Schema::CollectionSchema &schema)
{
    const std::string collection_meta_key=KeyConverter::collection_meta_key(collection_name);
    if(_collection_name_map.contains(collection_name)||_storage->contains(collection_meta_key)){
        return std::make_optional("The collection with name `"+collection_name+"` already exists");
    }
    //TODO

    return std::nullopt;
}

bool Skilo::CollectionManager::add_document(const Skilo::SegmentBuf &json_str)
{
//    try {
//        Document doc(_collection_id,_next_seq_id,json_str);
//        doc.write_to_storage(_storage);
//    }  catch (std::exception &e) {

//    }
}

} //namespace Skilo
