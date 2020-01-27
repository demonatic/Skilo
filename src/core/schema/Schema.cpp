#include "Schema.h"
#include <exception>
#include <iostream>

namespace Skilo{
namespace Schema{

CollectionSchema::CollectionSchema(const Document &schema)
{
    this->_fields=Field::create_field("$schema",schema.get_raw());
}

bool CollectionSchema::validate(const Document &document) const
{
    ValidateCode code=ValidateCode::OK;
    if(this->_fields){
        code=_fields->validate(document.get_raw());
    }
    return code==ValidateCode::OK;
}



} //namespace Schema
}//namespace Schema
