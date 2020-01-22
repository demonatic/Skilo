#ifndef KEYCONVERTER_HPP
#define KEYCONVERTER_HPP

#include <string>

namespace KeyConverter {

static constexpr const char *DOC_ID_KEY_PREFIX="$DC";
static constexpr const char *DOC_SEQ_KEY_PREFIX="$DS";
static constexpr const char *COLLECTION_META_KEY_PREFIX="$CM";
static constexpr const char *COLLECTION_NEXT_SEQ_KEY_PREFIX="$CS";

inline std::string doc_id_key(uint32_t collection_id,uint32_t doc_id){
    return std::to_string(collection_id)+'_'+DOC_ID_KEY_PREFIX+std::to_string(doc_id);
}

inline std::string doc_seq_key(uint32_t collection_id,uint32_t seq_id){
    return std::to_string(collection_id)+'_'+DOC_SEQ_KEY_PREFIX+std::to_string(seq_id);
}

inline std::string collection_next_seq_key(const std::string &collection_name){
    return collection_name+'_'+COLLECTION_NEXT_SEQ_KEY_PREFIX;
}

inline std::string collection_meta_key(const std::string &collection_name){
    return std::string(COLLECTION_META_KEY_PREFIX)+'_'+collection_name;
}

}



#endif // KEYCONVERTER_HPP
