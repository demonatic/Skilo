#ifndef ART_HPP
#define ART_HPP

#include "ArtNode.hpp"

namespace Art{

template <class T>
class ARTree{
public:
    ARTree():_root(nullptr){}
    ~ARTree();

    T* insert(const char *key,const T &val);


private:
    T* insert_impl(ArtNode *node,ArtNode::Ptr &node_ptr,const char *key,size_t key_len,size_t &depth,const T &val);

private:
    ArtNode *_root;
    uint64_t _size;
};

template<class T>
ARTree<T>::~ARTree(){

}

template<class T>
T *ARTree<T>::insert(const char *key, const T &val)
{
    T *old_val=insert_impl(_root,&_root,key,std::strlen(key),0,val);
    if(!old_val){
        _size++;
    }
    return old_val;
}

template<class T>
T *ARTree<T>::insert_impl(ArtNode *node,ArtNode::Ptr &node_ptr, const char *key, size_t key_len, size_t &depth, const T &val)
{
    if(!node){
        node_ptr=ArtLeaf<T>::make_leaf(key,key_len,val);
        set_leaf(node_ptr);
        return nullptr;
    }

    if(is_leaf(node)){
        //replace it with an InnerNode
        ArtLeaf<T> *leaf=ArtLeaf<T>::leaf_node_ptr(node);
        ArtNode4 *node4=new ArtNode4();

        size_t i;
        for(i=depth;key[i]!=leaf->key[i];i++){
            node4->_prefix[i-depth]=key[i];
        }
        node4->_prefix_len=i-depth;
        //TODO 处理len超过max_prefix_len
        depth+=node4->_prefix_len;
        node4->add_child(leaf->key[i],leaf);
        node4->add_child(key[i],node);
        node_ptr=node4;
        return nullptr;
    }
}



}



#endif // ART_HPP
