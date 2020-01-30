#include "Collection.h"

namespace Skilo {

Collection::Collection(const CollectionMeta &collection_meta,StorageService *storage_service):
    _schema(collection_meta),_storage_service(storage_service)
{
    _collection_name=collection_meta.get_collection_name();
    std::optional<uint32_t> next_seq_id=_storage_service->get_collection_next_seq_id(_collection_name);
    if(!next_seq_id){
        throw std::runtime_error("error when get collection \""+_collection_name+"\" next seq id ");
    }
    _next_seq_id=next_seq_id.value();
}

bool Collection::index_document(const Document &document)
{
    if(!validate_document(document)){
        return false;
    }
}

bool Collection::validate_document(const Document &document)
{
    return true;
}

} //namespace Skilo
