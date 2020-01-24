#ifndef COLLECTION_H
#define COLLECTION_H

#include <string>
#include <optional>
#include "Document.h"
#include "Schema.h"

namespace Skilo {

class Collection
{
public:
    Collection(uint32_t collection_id,const std::string &collection_name,Storage *storage);
    bool index_document(const Document &document);

    /// @brief check whether document adhere to the schema
    bool validate_document(const Document &document);
    
private:
    uint32_t _collection_id;
    uint32_t _next_seq_id;
    std::string _collection_name;

    Storage *_storage;
};


} //namespace Skilo

#endif // COLLECTION_H


