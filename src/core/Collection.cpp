#include "Collection.h"

namespace Skilo {

Collection::Collection(uint32_t collection_id,const std::string &collection_name, Storage *storage):
   _collection_id(collection_id),_collection_name(collection_name),_storage(storage)
{

    std::string next_seq_key;
    Storage::Status next_seq_status=_storage->get(KeyConverter::collection_next_seq_key(collection_name),next_seq_key);
    if(next_seq_status==Storage::ERROR){

    }
}

bool Collection::index_document(const Skilo::Document &document)
{
    if(!validate_document(document)){
        return false;
    }
}

bool Collection::validate_document(const Skilo::Document &document)
{
    return true;
}

} //namespace Skilo
