#ifndef KEYCONVERTER_HPP
#define KEYCONVERTER_HPP

#include <string>

namespace KeyConverter {

static constexpr const char *DOC_ID_KEY_PREFIX="$DC";
static constexpr const char *DOC_SEQ_KEY_PREFIX="$DS";
static constexpr const char *COLLECTION_META_KEY_PREFIX="$MT";
static constexpr const char *COLLECTION_NEXT_SEQ_KEY_PREFIX="$CS";
static constexpr const char *NEXT_COLLECTION_ID="$CI";


std::string serialize_uint32_t(const uint32_t num){
    uint8_t encoded[4];
    encoded[0]=(num>>24)&0xff;
    encoded[1]=(num>>16)&0xff;
    encoded[2]=(num>>8)&0xff;
    encoded[3]=num&0xff;
    return std::string(encoded,encoded+4);
}

uint32_t deserialize_to_uint32_t(const std::string &str){
    return (((str[0]&0xff)<<24)|((str[1]&0xff)<<16)|((str[2]&0xff)<<8)|(str[3]&0xff));
}

/// @brief key prefix for a collection's each documents
std::string doc_seq_key_prefix(const uint32_t collection_id){
    return serialize_uint32_t(collection_id)+'_'+DOC_SEQ_KEY_PREFIX+'_';
}

std::string doc_id_key_prefix(const uint32_t collection_id){
    return serialize_uint32_t(collection_id)+'_'+DOC_ID_KEY_PREFIX+'_';
}

std::string collection_meta_prefix(){
     return std::string(COLLECTION_META_KEY_PREFIX)+'_';
}

/// @brief user-assigns doc id for a collection's one single document
std::string doc_id_key(const uint32_t collection_id,const uint32_t doc_id){
    return doc_id_key_prefix(collection_id)+serialize_uint32_t(doc_id);
}

/// @brief sequence key for a collection's one single document
std::string doc_seq_key(const uint32_t collection_id,const uint32_t seq_id){
    return doc_seq_key_prefix(collection_id)+serialize_uint32_t(seq_id);
}

std::string collection_next_seq_key(const uint32_t collection_id){
    return serialize_uint32_t(collection_id)+'_'+COLLECTION_NEXT_SEQ_KEY_PREFIX;
}

/// @brief key for a collection's meta data
std::string collection_meta_key(const std::string &collection_name){
    return collection_meta_prefix()+collection_name;
}

/// @brief global key for next create collection's id
const char* next_collection_id_key(){
    return NEXT_COLLECTION_ID;
}


}



#endif // KEYCONVERTER_HPP
