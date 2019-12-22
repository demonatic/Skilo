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

struct InnerNode:public ArtNode
{
    enum NodeType:uint8_t{
        Node4,
        Node16,
        Node48,
        Node256
    };

    InnerNode(NodeType node_type):_node_type(node_type),_prefix_len(0){}

    /// @return the first position that mismatch, return prefix_len if match all
    int check_prefix(const char *partial_key,int key_len){
        for(int i=0;i<_prefix_len&&i<key_len;i++){
            if(_prefix[i]!=partial_key[i]){
                return i;
            }
        }
        return _prefix_len;
    }

    NodeType _node_type;
    uint8_t _prefix_len;
    static constexpr size_t MAX_PREFIX_LEN=13;
    unsigned char _prefix[MAX_PREFIX_LEN];
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
    static constexpr char InvalidIndex=48;

    ArtNode48():InnerNode(Node48),_num_children(0),_child_indexs(),_children(){}

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


template<class T>
struct ArtLeaf:public ArtNode{
    static ArtLeaf* make_leaf(const unsigned char *key,size_t key_len,const T &value){
        ArtLeaf *leaf=new ArtLeaf;
        leaf->key=malloc(key_len);
        std::memcpy(leaf->key,key,key_len);
        leaf->key_len=key_len;
        leaf->data=value;
    }

    ArtLeaf* leaf_node_ptr(ArtNode::Ptr ptr){
       return reinterpret_cast<ArtLeaf*>(reinterpret_cast<uintptr_t>(ptr)&(~0x1ul));
    }

    const char *key; //TODO should contain '\0'
    size_t key_len;
    T data;
};

bool is_leaf(ArtNode::Ptr ptr){
    return reinterpret_cast<uintptr_t>(ptr)&0x1;
}

void set_leaf(ArtNode::Ptr &ptr){
    ptr=reinterpret_cast<ArtNode::Ptr>((reinterpret_cast<uintptr_t>(ptr)|0x1));
}


}


#endif // ARTNODE_H
