#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <optional>
#include "Document.h"
#include "Schema.h"

class Collection
{
public:
    Collection(uint32_t collection_id,const std::string &collection_name,Storage *storage);
    bool add_document(const SegmentBuf &json_str);
    
    
private:
    uint32_t _collection_id;
    uint32_t _next_seq_id;
    std::string _collection_name;

    Storage *_storage;
};

#endif // COLLECTION_H
