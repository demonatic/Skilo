#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include "Document.h"

class Collection
{
public:
    Collection();
    bool add_document(const SegmentBuf &json_str);
    
    
private:
    uint32_t _collection_id;
};

#endif // COLLECTION_H
