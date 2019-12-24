#ifndef ARTNODE_H
#define ARTNODE_H

#include <algorithm>
#include <cstring>
#include <assert.h>
#if defined(__i386__) || defined(__amd64__)
#include <emmintrin.h>
#endif

namespace Art {

struct ArtNode{ using Ptr=ArtNode*; };

template<class T>
struct ArtLeaf:public ArtNode{

    static ArtLeaf* make_leaf(const unsigned char *key,size_t key_len,const T &value){
        ArtLeaf *leaf=new ArtLeaf;
        leaf->key=calloc(key_len+1,1); //TODO ok?
        std::memcpy(leaf->key,key,key_len);
        leaf->key_len=key_len;
        leaf->data=new T(value);
    }

    bool key_match(const unsigned char *key,size_t key_len){
        if(key_len!=this->key_len){
            return false;
        }
        return std::memcmp(key,this->key,key_len);
    }

    ArtLeaf* leaf_node_ptr(ArtNode::Ptr ptr){
       return reinterpret_cast<ArtLeaf*>(reinterpret_cast<uintptr_t>(ptr)&(~0x1ul));
    }

    const char *key; //a full key, should contain '\0'
    size_t key_len;
    T *data;
};

bool is_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<uintptr_t>(ptr)&0x1;
}

ArtNode::Ptr as_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<ArtNode::Ptr>((reinterpret_cast<uintptr_t>(ptr)|0x1));
}

ArtNode *find_first_leaf(ArtNode *node);

struct InnerNode:public ArtNode
{
    enum NodeType:uint8_t{
        Node4,
        Node16,
        Node48,
        Node256
    };

    InnerNode(NodeType node_type):_node_type(node_type),_prefix_len(0){}

    /// @return the length match [0,_prefinx_len] and the prefix's key
    template<class T>
    std::pair<uint32_t,const unsigned char *> check_prefix(const char *key,uint32_t depth,uint32_t key_len){
        uint32_t match_len=0;
        for(;match_len<std::min(key_len-depth,std::min(_prefix_len,PREFIX_VEC_LEN));match_len++){
            if(_prefix[match_len]!=key[depth+match_len]){
                return {match_len,_prefix};
            }
        }
        if(_prefix_len>PREFIX_VEC_LEN)
            return {match_len,_prefix};

        //if the prefix is larger than PREFIX_VEC_LEN, we must find the remaining key from a leaf to compare
        ArtLeaf<T> *leaf=find_first_leaf(this);
        if(_prefix_len>PREFIX_VEC_LEN){
            //resume compare of the remaining key doesn't stored in the inner node
            for(;match_len<min(_prefix_len,min(key_len,leaf->key_len)-depth);match_len++){
                if(leaf->key[depth+match_len]!=key[depth+match_len]){
                    return {match_len,leaf->key+depth};
                }
            }
        }
        return {match_len,leaf->key+depth};
    }

    NodeType _node_type;
    uint32_t _prefix_len; //may larger than PREFIX_VEC_LEN
    static constexpr uint32_t PREFIX_VEC_LEN=8;
    unsigned char _prefix[PREFIX_VEC_LEN];
};


struct ArtNode256:public InnerNode{
public:
    ArtNode256():InnerNode(InnerNode::Node256),_num_children(0),_children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode *child=_children[key];
        return child!=nullptr?&_children[key]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        assert(_num_children<256);
        _children[key]=child_node;
        _num_children++;
    }

    bool is_full(){
        return _num_children==256;
    }

    bool can_shrink(){
        return _num_children<=48;
    }

    uint16_t _num_children; //0-256
    ArtNode *_children[256];
};

struct ArtNode48:public InnerNode{
    static constexpr unsigned char InvalidIndex=48;

    ArtNode48():InnerNode(Node48),_num_children(0),_child_indexs{InvalidIndex},_children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
        int index=_child_indexs[key];
        return index!=InvalidIndex?&_children[index]:nullptr;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        assert(_num_children<48);
        _child_indexs[key]=_num_children;
        _children[_num_children]=child_node;
        _num_children++;
    }

    bool is_full(){
        return _num_children==48;
    }

    bool can_shrink(){
        return _num_children<=16;
    }

    uint8_t _num_children;
    unsigned char _child_indexs[256];
    ArtNode *_children[48];
};

struct ArtNode16:public InnerNode{
public:
    ArtNode16():InnerNode(InnerNode::Node16),_num_children(0),_keys(),_children(){}

    ArtNode::Ptr* find_child(const unsigned char key){
#if defined(__i386__)||defined (__amd64__)
        int match_result = _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(key),_mm_loadu_si128((__m128i *)_keys)));
         // build a mask to select only the first _num_children values from the comparison
        int mask=(1<<_num_children)-1;
        int bit_field=match_result&mask;
         // find the index of the first '1' in the bitfield by counting the leading zeros.
        int index=__builtin_ctz(bit_field);
        return bit_field!=0?&_children[index]:nullptr;
#else
        int low=0,mid,high=_num_children;
        while(low<high){
            mid=(low+high)/2;
            if(key<_keys[mid]){
                high=mid;
            }else if(key>_keys[mid]){
                low=mid+1;
            }else{
                return &_children[mid];
            }
        }
        return nullptr;
#endif
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        assert(_num_children<16);
#if defined(__i386__)||defined (__amd64__)
        _keys[_num_children]=key;
        _children[_num_children]=child_node;
        _num_children++;
#else
        int i; //insert position
        for(i=_num_children;i>0;i--){
            if(key>_keys[i-1]){
               break;
            }
            _children[i]=_children[i-1];
            _keys[i]=_keys[i-1];
        }
        _keys[i-1]=key;
        _children[i-1]=child_node;
        _num_children++;

#endif
    }

    bool is_full(){
        return _num_children==16;
    }

    bool can_shrink(){
        return _num_children<=4;
    }

    uint8_t _num_children;
    unsigned char _keys[16]; //sorted if __i386__ or __amd64__ is not defined
    ArtNode *_children[16];
};


struct ArtNode4:public InnerNode{

    ArtNode4():InnerNode(InnerNode::Node4),_num_children(0),_keys(),_children(){}

    /// @return child_node pointer with the given 'key', nullptr if the corresponding child of the 'key' is not found
    ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode::Ptr *child=nullptr;
        for(int i=0;i<4;i++){
            if(_keys[i]==key){
                child=&_children[i];
            }
        }
        return child;
    }

    void add_child(const unsigned char key,ArtNode *child_node){
        assert(_num_children<4);
        _keys[_num_children]=key;
        _children[_num_children]=child_node;
        _num_children++;
    }

    bool is_full(){
        return _num_children==4;
    }

    bool can_shrink(){
        return false;
    }

    //NEED THIS?
    InnerNode *expand(){
        ArtNode16 *new_node=new ArtNode16();
        std::memcpy(new_node->_prefix,this->_prefix,_prefix_len);
        new_node->_prefix_len=this->_prefix_len;
        new_node->_num_children=this->_num_children;
        std::memcpy(new_node->_keys,this->_keys,sizeof(_keys));
        std::memcpy(new_node->_children,this->_children,sizeof(_children));
#if !defined(__i386__)&&!defined(__amd64__)
        std::sort(new_node->_keys,new_node->_keys+sizeof(_keys));
        std::sort(new_node->_children,new_node->_children+sizeof(_children));
#endif
        delete this;
        return new_node;
    }

    uint8_t _num_children;
    unsigned char _keys[4];
    ArtNode *_children[4];
};

ArtNode *find_first_leaf(ArtNode *node){
    if(!node)
        return nullptr;

    if(is_leaf(node))
        return as_leaf(node);

    InnerNode *inner_node=static_cast<InnerNode*>(node);
    switch (inner_node->_node_type){
        case InnerNode::Node4:{
            return find_first_leaf(static_cast<ArtNode4*>(inner_node)->_children[0]);
        }
        case InnerNode::Node16:{
            return find_first_leaf(static_cast<ArtNode16*>(inner_node)->_children[0]);
        }
        case InnerNode::Node48:{
            return find_first_leaf(static_cast<ArtNode48*>(inner_node)->_children[0]);
        }
        case InnerNode::Node256:{
            int idx=0;
            ArtNode256 *node=static_cast<ArtNode256*>(inner_node);
            while(node->_children[idx]) idx++;
            return find_first_leaf(node->_children[idx]);
        }
    }
}

InnerNode *as_inner_node(ArtNode *ptr){
    return static_cast<InnerNode*>(ptr);
}

}


#endif // ARTNODE_H
