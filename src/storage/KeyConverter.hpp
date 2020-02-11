#ifndef KEYCONVERTER_HPP
#define KEYCONVERTER_HPP

#include <string>

namespace KeyConverter {

static constexpr const char *DOC_ID_KEY_PREFIX="$DC";
static constexpr const char *DOC_SEQ_KEY_PREFIX="$DS";
static constexpr const char *COLLECTION_META_KEY_PREFIX="$MT";
static constexpr const char *COLLECTION_NEXT_SEQ_KEY_PREFIX="$CS";
static constexpr const char *NEXT_COLLECTION_ID="$CI";

inline std::string doc_seq_key_prefix(const uint32_t collection_id){
    return std::to_string(collection_id)+'_'+DOC_SEQ_KEY_PREFIX+'_';
}

inline std::string collection_meta_prefix(){
     return std::string(COLLECTION_META_KEY_PREFIX)+'_';
}

inline std::string doc_id_key(const uint32_t collection_id,const uint32_t doc_id){
    return std::to_string(collection_id)+'_'+DOC_ID_KEY_PREFIX+'_'+std::to_string(doc_id);
}

inline std::string doc_seq_key(const uint32_t collection_id,const uint32_t seq_id){
    return doc_seq_key_prefix(collection_id)+std::to_string(seq_id);
}

inline std::string collection_next_seq_key(const uint32_t collection_id){
    return std::to_string(collection_id)+'_'+COLLECTION_NEXT_SEQ_KEY_PREFIX;
}

inline std::string collection_meta_key(const std::string &collection_name){
    return collection_meta_prefix()+collection_name;
}

inline const char* next_collection_id_key(){
    return NEXT_COLLECTION_ID;
}

}



#endif // KEYCONVERTER_HPP
