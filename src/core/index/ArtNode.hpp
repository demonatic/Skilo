#ifndef ARTNODE_H
#define ARTNODE_H

#include <algorithm>
#include <cstring>
#include <emmintrin.h>
#include <assert.h>
#include <iostream>
using namespace std;

namespace Art {

struct ArtNode{ using Ptr=ArtNode*; };

template<class T>
struct ArtLeaf:public ArtNode{
    ~ArtLeaf(){
        if(!is_key_local()){
            free(key);
        }
        delete data;
    }

    bool is_key_local(){
        return this->key_len<key_local_capacity;
    }

    unsigned char* get_key(){
         return is_key_local()?key_local:this->key;
    }

    static ArtLeaf* make_leaf(const unsigned char *key,size_t key_len,T *value){
        ArtLeaf *leaf=new ArtLeaf;
        leaf->key_len=key_len;
        leaf->data=value;
        if(leaf->is_key_local()){
             std::memcpy(leaf->key_local,key,key_len+1); //1 for '\0'
        }
        else{
            leaf->key=static_cast<unsigned char *>(malloc(key_len+1));
            std::memcpy(leaf->key,key,key_len+1); //1 for '\0'
        }
        return leaf;
    }

    static ArtLeaf<T>* as_leaf_node(ArtNode::Ptr ptr){
       return reinterpret_cast<ArtLeaf<T>*>(reinterpret_cast<uintptr_t>(ptr)&(~0x1ul));
    }

    bool key_match(const unsigned char *key,size_t key_len){
        if(key_len!=this->key_len){
            return false;
        }
        return std::memcmp(key,get_key(),key_len)==0;
    }

    static constexpr uint32_t key_local_capacity=16;
    union{
        unsigned char *key; //a full key, should contain '\0'
        unsigned char key_local[key_local_capacity];
    };
    uint32_t key_len;
    T *data;
};

inline bool is_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<uintptr_t>(ptr)&0x1;
}

inline ArtNode::Ptr store_as_leaf(ArtNode::Ptr ptr){
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
    uint32_t prefix_len; //0-n, may larger than PREFIX_VEC_LEN
    static constexpr uint32_t PREFIX_VEC_LEN=10;
    unsigned char prefix[PREFIX_VEC_LEN];
};

struct ArtNode4;
struct ArtNode16;
struct ArtNode48;
struct ArtNode256;

struct ArtNode4:public InnerNode{

    ArtNode4():InnerNode(InnerNode::Node4),num_children(0),keys(),children(){}

    /// @return child_node pointer with the given 'key', nullptr if the corresponding child of the 'key' is not found
    ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode::Ptr *child=nullptr;
        for(int i=0;i<num_children;i++){
            if(keys[i]==key){
                child=&children[i];
            }
        }
        return child;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
         //find correct insert position to keep it sorted
        size_t insert_pos=0;
        while(insert_pos<num_children&&key>=keys[insert_pos]) insert_pos++;

        size_t move_size=num_children-insert_pos;
        std::memmove(keys+insert_pos+1,keys+insert_pos,move_size);
        std::memmove(children+insert_pos+1,children+insert_pos,move_size*sizeof(ArtNode*));

        keys[insert_pos]=key;
        children[insert_pos]=child_node;
        num_children++;
    }
    
    void remove_child(ArtNode::Ptr *child_addr){
        size_t pos=child_addr-children;
        size_t move_len=num_children-pos-1;
        std::memmove(keys+pos,keys+pos+1,move_len);
        std::memmove(children+pos,children+pos+1,move_len*sizeof(ArtNode*));
        num_children--;
    }

    bool is_full(){
        return num_children==4;
    }

    ArtNode16 *expand();

    uint8_t num_children;
    unsigned char keys[4];
    ArtNode *children[4];
};

struct ArtNode16:public InnerNode{
public:
    ArtNode16():InnerNode(InnerNode::Node16),num_children(0),keys(),children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
        int match_result = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(key),_mm_loadu_si128((__m128i *)keys)));
         // build a mask to select only the first _num_children values from the comparison
        int mask=(1<<num_children)-1;
        int bit_field=match_result&mask;
         // find the index of the first '1' in the bitfield by counting the tailing zeros.
        int index=__builtin_ctz(bit_field);
        return bit_field!=0?&children[index]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        size_t insert_pos;
        for(int i=num_children-1;;i--){
            if(key<keys[i]&&i>=0){
                keys[i+1]=keys[i];
                children[i+1]=children[i];
            }
            else{
                insert_pos=i+1;
                break;
            }
        }
        keys[insert_pos]=key;
        children[insert_pos]=child_node;
        num_children++;
    }
    
    void remove_child(ArtNode::Ptr *child_addr){
        size_t erase_pos=child_addr-children;
        size_t move_size=num_children-erase_pos-1;
        std::memmove(keys+erase_pos,keys+erase_pos+1,move_size);
        std::memmove(children+erase_pos,children+erase_pos+1,move_size*sizeof(ArtNode*));
        num_children--;
    }

    bool is_full(){
        return num_children==16;
    }

    bool can_shrink(){
        return num_children<=3;
    }

    ArtNode48 *expand();
    ArtNode4 *shrink();

    uint8_t num_children;
    unsigned char keys[16];
    ArtNode *children[16];
};

struct ArtNode48:public InnerNode{
    ArtNode48():InnerNode(Node48),num_children(0),child_indexs(),children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
        unsigned char index=child_indexs[key];
        return index?&children[index-1]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        int pos=0;
        while(children[pos]) pos++;
        child_indexs[key]=pos+1;
        children[pos]=child_node;
        num_children++;
    }
    
    void remove_child(unsigned char key){
        int index=child_indexs[key];
        child_indexs[key]=0;
        children[index-1]=nullptr;
        num_children--;
    }

    bool is_full(){
        return num_children==48;
    }

    bool can_shrink(){
        return num_children<=12;
    }

    ArtNode256 *expand();
    ArtNode16 *shrink();

    uint8_t num_children;
    unsigned char child_indexs[256]; //index=k represents at children[k-1]
    ArtNode *children[48];
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
    
    void remove_child(unsigned char key){
        children[key]=nullptr;
        num_children--;
    }

    bool is_full(){
        return num_children==256;
    }

    bool can_shrink(){
        return num_children<=40;
    }

    ArtNode48 *shrink();

    uint16_t num_children; //49-256
    ArtNode *children[256];
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
   return ArtLeaf<T>::as_leaf_node(find_first_leaf_impl(node));
}

inline InnerNode *as_inner_node(ArtNode *ptr){
    return static_cast<InnerNode*>(ptr);
}

inline ArtNode16 *ArtNode4::expand()
{
    ArtNode16 *new_node=new ArtNode16();
    std::memcpy(new_node->prefix,this->prefix,prefix_len);
    new_node->prefix_len=this->prefix_len;
    new_node->num_children=this->num_children;
    std::memcpy(new_node->keys,this->keys,sizeof(keys));
    std::memcpy(new_node->children,this->children,sizeof(children));
    delete this;
    return new_node;
}

inline ArtNode4 *ArtNode16::shrink()
{
    ArtNode4 *new_node=new ArtNode4;
    std::memcpy(new_node->prefix,this->prefix,prefix_len);
    new_node->prefix_len=this->prefix_len;
    new_node->num_children=this->num_children;
    std::memcpy(new_node->keys,this->keys,this->num_children*sizeof(unsigned char));
    std::memcpy(new_node->children,this->children,this->num_children*sizeof(ArtNode*));
    delete this;
    return new_node;
}

inline ArtNode48 *ArtNode16::expand()
{
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

inline ArtNode16 *ArtNode48::shrink()
{
    ArtNode16 *new_node=new ArtNode16();
    std::memcpy(new_node->prefix,this->prefix,prefix_len);
    new_node->prefix_len=this->prefix_len;
    new_node->num_children=this->num_children;
    int pos=0;
    for(int i=0;i<256;i++){
         unsigned char index=child_indexs[i];
         if(index){
             new_node->keys[pos]=static_cast<unsigned char>(i);
             new_node->children[pos]=this->children[index-1];
             pos++;
         }
    }
    delete this;
    return new_node;
}

inline ArtNode256 *ArtNode48::expand()
{
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

inline ArtNode48 *ArtNode256::shrink()
{
    ArtNode48 *new_node=new ArtNode48();
    std::memcpy(new_node->prefix,this->prefix,prefix_len);
    new_node->prefix_len=this->prefix_len;
    for(int i=0;i<256;i++){
        if(this->children[i]){
            unsigned char key=static_cast<unsigned char>(i);
            new_node->add_child(key,this->children[i]);
        }
    }
    delete this;
    return new_node;
}

inline ArtNode4 *as_node4(InnerNode *inner_node){
    assert(inner_node->type==InnerNode::Node4);
    return static_cast<ArtNode4*>(inner_node);
}

inline ArtNode16 *as_node16(InnerNode *inner_node){
    assert(inner_node->type==InnerNode::Node16);
    return static_cast<ArtNode16*>(inner_node);
}

inline ArtNode48 *as_node48(InnerNode *inner_node){
    assert(inner_node->type==InnerNode::Node48);
    return static_cast<ArtNode48*>(inner_node);
}

inline ArtNode256 *as_node256(InnerNode *inner_node){
    assert(inner_node->type==InnerNode::Node256);
    return static_cast<ArtNode256*>(inner_node);
}
}


#endif // ARTNODE_H
