#ifndef ARTNODE_H
#define ARTNODE_H

#include <algorithm>
#include <cstring>
#include <assert.h>
#if defined(__i386__) || defined(__amd64__)
#include <emmintrin.h>
#endif

#include <iostream>
using namespace std;

namespace Art {

struct ArtNode{ using Ptr=ArtNode*; };

template<class T>
struct ArtLeaf:public ArtNode{

    static ArtLeaf* make_leaf(const unsigned char *key,size_t key_len,const T &value){
        ArtLeaf *leaf=new ArtLeaf;
        leaf->key=static_cast<unsigned char *>(malloc(key_len+1));
        std::memcpy(leaf->key,key,key_len+1); //1 for '\0'
        leaf->key_len=key_len;
        leaf->data=new T(value);
        return leaf;
    }

    static ArtLeaf<T>* leaf_node_ptr(ArtNode::Ptr ptr){
       return reinterpret_cast<ArtLeaf<T>*>(reinterpret_cast<uintptr_t>(ptr)&(~0x1ul));
    }

    bool key_match(const unsigned char *query_key,size_t key_len){
        if(key_len!=this->key_len){
            return false;
        }
        auto *cmp_key=reinterpret_cast<const unsigned char *>(query_key);
        return std::memcmp(cmp_key,this->key,key_len)==0;
    }

    unsigned char *key; //a full key, should contain '\0'
    uint32_t key_len;
    T *data;
};

inline bool is_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<uintptr_t>(ptr)&0x1;
}

inline ArtNode::Ptr as_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<ArtNode::Ptr>((reinterpret_cast<uintptr_t>(ptr)|0x1));
}

template<class T>
ArtLeaf<T> *find_first_leaf(ArtNode *node);

struct InnerNode:public ArtNode
{
    enum NodeType:uint8_t{
        Node4,
        Node16,
        Node48,
        Node256
    };

    InnerNode(NodeType node_type):type(node_type),prefix_len(0){}

    /// @return the length match [0,_prefinx_len] and the prefix's key
    template<class T>
    std::pair<uint32_t,const unsigned char *> check_prefix(const unsigned char *key,uint32_t depth,uint32_t key_len){
        uint32_t match_len=0;
        for(;match_len<std::min(key_len-depth,std::min(prefix_len,PREFIX_VEC_LEN));match_len++){
            if(this->prefix[match_len]!=key[depth+match_len]){
                return {match_len,this->prefix};
            }
        }
        if(prefix_len<=PREFIX_VEC_LEN)
            return {match_len,this->prefix};

        //if the prefix is larger than PREFIX_VEC_LEN, we must find the remaining key from a leaf to compare
        ArtLeaf<T> *leaf=find_first_leaf<T>(this);
        if(prefix_len>PREFIX_VEC_LEN){
            //resume compare of the remaining key doesn't stored in the inner node
            for(;match_len<std::min(prefix_len,key_len)-depth;match_len++){
                if(leaf->key[depth+match_len]!=key[depth+match_len]){
                    return {match_len,leaf->key+depth};
                }
            }
        }
        return {match_len,leaf->key+depth};
    }

    NodeType type;
    uint32_t prefix_len; //may larger than PREFIX_VEC_LEN
    static constexpr uint32_t PREFIX_VEC_LEN=10;
    unsigned char prefix[PREFIX_VEC_LEN];
};


struct ArtNode256:public InnerNode{
public:
    ArtNode256():InnerNode(InnerNode::Node256),num_children(0),children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode *child=children[key];
        return child!=nullptr?&children[key]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        children[key]=child_node;
        num_children++;
    }

    bool is_full(){
        return num_children==256;
    }

    bool can_shrink(){
        return num_children<=48;
    }

    uint16_t num_children; //0-256
    ArtNode *children[256];
};

struct ArtNode48:public InnerNode{
    ArtNode48():InnerNode(Node48),num_children(0),child_indexs(),children(){

    }

    ArtNode::Ptr* find_child(const unsigned char key){
        unsigned char index=child_indexs[key];
        return index?&children[index-1]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        assert(num_children<48);
        child_indexs[key]=num_children+1;
        children[num_children]=child_node;
        num_children++;
    }

    bool is_full(){
        return num_children==48;
    }

    bool can_shrink(){
        return num_children<=16;
    }

    InnerNode *expand(){
        ArtNode256 *new_node=new ArtNode256();
        std::memcpy(new_node->prefix,this->prefix,prefix_len);
        new_node->prefix_len=this->prefix_len;
        new_node->num_children=this->num_children;
        for(int i=0;i<256;i++){
            unsigned char index=child_indexs[i];
            if(index){
                new_node->children[i]=this->children[index-1];
            }
        }
        delete this;
        return new_node;
    }

    uint8_t num_children;
    unsigned char child_indexs[256]; //index=k则位于children[k-1]处
    ArtNode *children[48];
};

struct ArtNode16:public InnerNode{
public:
    ArtNode16():InnerNode(InnerNode::Node16),num_children(0),keys(),children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
#if defined(__i386__)||defined (__amd64__)
        int match_result = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(key),_mm_loadu_si128((__m128i *)keys)));
         // build a mask to select only the first _num_children values from the comparison
        int mask=(1<<num_children)-1;
        int bit_field=match_result&mask;
         // find the index of the first '1' in the bitfield by counting the leading zeros.
        int index=__builtin_ctz(bit_field);
        return bit_field!=0?&children[index]:nullptr;

#else
        int low=0,mid,high=num_children;
        while(low<high){
            mid=(low+high)/2;
            if(key<keys[mid]){
                high=mid;
            }else if(key>keys[mid]){
                low=mid+1;
            }else{
                return &children[mid];
            }
        }
        return nullptr;
#endif
    }

    void add_child(const unsigned char key,ArtNode *child_node){
#if defined(__i386__)||defined (__amd64__)
        keys[num_children]=key;
        children[num_children]=child_node;
        num_children++;
#else
        int i=0; //insert position
        if(num_children){
            for(i=num_children;i>0;i--){
                if(key>keys[i-1]){
                   break;
                }
                children[i]=children[i-1];
                keys[i]=keys[i-1];
            }
        }
        keys[i]=key;
        children[i]=child_node;
        num_children++;

#endif
    }

    bool is_full(){
        return num_children==16;
    }

    bool can_shrink(){
        return num_children<=4;
    }

    InnerNode *expand(){
        ArtNode48 *new_node=new ArtNode48();
        std::memcpy(new_node->prefix,this->prefix,prefix_len);
        new_node->prefix_len=this->prefix_len;
        new_node->num_children=this->num_children;
        std::memcpy(new_node->children,this->children,sizeof(children));
        for(uint8_t i=0;i<num_children;i++){
            new_node->child_indexs[this->keys[i]]=i+1;
        }

        delete this;
        return new_node;
    }

    uint8_t num_children;
    unsigned char keys[16]; //sorted if __i386__ or __amd64__ is not defined
    ArtNode *children[16];
};


struct ArtNode4:public InnerNode{

    ArtNode4():InnerNode(InnerNode::Node4),num_children(0),keys(),children(){}

    /// @return child_node pointer with the given 'key', nullptr if the corresponding child of the 'key' is not found
    ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode::Ptr *child=nullptr;
        for(int i=0;i<num_children;i++){
            if(keys[i]==key){
                child=&children[i];
                break;
            }
        }
        return child;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        keys[num_children]=key;
        children[num_children]=child_node;
        num_children++;
    }

    bool is_full(){
        return num_children==4;
    }

    bool can_shrink(){
        return false;
    }

    InnerNode *expand(){
        ArtNode16 *new_node=new ArtNode16();
        std::memcpy(new_node->prefix,this->prefix,prefix_len);
        new_node->prefix_len=this->prefix_len;
        new_node->num_children=this->num_children;
        std::memcpy(new_node->keys,this->keys,4);
        std::memcpy(new_node->children,this->children,4*sizeof(ArtNode4*));
#if !defined(__i386__)&&!defined(__amd64__)
        std::sort(new_node->_keys,new_node->_keys+sizeof(_keys));
        std::sort(new_node->_children,new_node->_children+sizeof(_children));
#endif
        return new_node;
    }

    uint8_t num_children;
    unsigned char keys[4];
    ArtNode *children[4];
};

inline ArtNode *find_first_leaf_impl(ArtNode *node){
    if(!node)
        return nullptr;

    if(is_leaf(node))
        return node;

    InnerNode *inner_node=static_cast<InnerNode*>(node);
    switch (inner_node->type){
        case InnerNode::Node4:{
            return find_first_leaf_impl(static_cast<ArtNode4*>(inner_node)->children[0]);
        }
        case InnerNode::Node16:{
            return find_first_leaf_impl(static_cast<ArtNode16*>(inner_node)->children[0]);
        }
        case InnerNode::Node48:{
            return find_first_leaf_impl(static_cast<ArtNode48*>(inner_node)->children[0]);
        }
        case InnerNode::Node256:{
            int idx=0; //TODO start from high possible position
            ArtNode256 *node=static_cast<ArtNode256*>(inner_node);
            while(node->children[idx]) idx++;
            return find_first_leaf_impl(node->children[idx]);
        }
    }
    return nullptr;
}

template<class T>
inline ArtLeaf<T> *find_first_leaf(ArtNode *node){
   return ArtLeaf<T>::leaf_node_ptr(find_first_leaf_impl(node));
}

inline InnerNode *as_inner_node(ArtNode *ptr){
    return static_cast<InnerNode*>(ptr);
}

}


#endif // ARTNODE_H
