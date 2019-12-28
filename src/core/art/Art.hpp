#ifndef ART_HPP
#define ART_HPP

#include "ArtNode.hpp"

namespace Art{

template <class T>
class ARTree{
public:
    ARTree():_root(nullptr),_size(0){}
    ~ARTree();

    /// @param key can be any byte sequence doesn't have '\0' except it's end
    T* insert(const char *key,size_t key_len,const T &val);
    /// @return nullptr if not find, otherwise return the pointer to the value
    T *find(const char *key,size_t key_len);

private:
    ArtNode::Ptr *find_child(InnerNode *node,const unsigned char key);
    void add_child(InnerNode *node,ArtNode::Ptr *node_ptr,const unsigned char child_key,ArtNode *child);

    T* insert_impl(ArtNode *node,ArtNode::Ptr *node_ptr,const unsigned char *key,size_t key_len,size_t depth,const T &val);

private:
    ArtNode *_root;
    uint64_t _size;
};


template<class T>
ARTree<T>::~ARTree(){

}

template<class T>
T *ARTree<T>::insert(const char *key, size_t key_len,const T &val)
{
    T *old_val=insert_impl(_root,&_root,reinterpret_cast<const unsigned char*>(key),key_len,0,val);
    if(!old_val){
        _size++;
    }
    return old_val;
}

/// Optimistic Search
/// InnerNode lookups just skip the node's prefix without comparing them.
/// Instead, when a lookup arrives at a leaf its key must be compared to the search
/// key to ensure that no “wrong turn” was taken
template<class T>
T *ARTree<T>::find(const char *key,size_t key_len)
{
//    cout<<"@find key="<<key<<"  key_len="<<key_len<<endl;
    ArtNode *node=_root;
    size_t depth=0;
    while(node){
        if(is_leaf(node)){
//            cout<<"is leaf"<<endl;
            ArtLeaf<T> *leaf=ArtLeaf<T>::leaf_node_ptr(node);
            return leaf->key_match(reinterpret_cast<const unsigned char*>(key),key_len)?leaf->data:nullptr;
        }

        InnerNode *inner_node=as_inner_node(node);
        depth+=inner_node->prefix_len;
//        std::cout<<"depth="<<depth<<" *key="<<key[depth]<<endl;
        ArtNode::Ptr *child=this->find_child(inner_node,key[depth]);

        node=child?*child:nullptr;
        depth++;
    }
    return nullptr;
}

template<class T>
T *ARTree<T>::insert_impl(ArtNode *node,ArtNode::Ptr *node_ptr, const unsigned char *key, size_t key_len, size_t depth, const T &val)
{
    if(!node){
        *node_ptr=as_leaf(ArtLeaf<T>::make_leaf(key,key_len,val));
        return nullptr;
    }

    if(is_leaf(node)){
        ArtLeaf<T> *leaf=ArtLeaf<T>::leaf_node_ptr(node);
        if(leaf->key_match(key,key_len)){ //the key we are updating already exists
            return leaf->data;
        }

        /// two leaves(origin leaf node and the inserted one) become two children of a node4
        ArtNode4 *new_node=new ArtNode4();

        size_t match_cur; //the longest common prefix length of key in the leaf node and 'key'
        for(match_cur=depth;match_cur-depth<InnerNode::PREFIX_VEC_LEN&&key[match_cur]==leaf->key[match_cur];match_cur++){
            new_node->prefix[match_cur-depth]=key[match_cur];
        }
        // the prefix_len field may longer than the actual prefix array
        new_node->prefix_len=match_cur-depth;
        new_node->add_child(leaf->key[match_cur],as_leaf(node));

        ArtLeaf<T> *insert_leaf=ArtLeaf<T>::make_leaf(key,key_len,val);
        new_node->add_child(key[match_cur],as_leaf(insert_leaf));

        *node_ptr=new_node;
        return nullptr;
    }

    InnerNode *inner_node=as_inner_node(node);
    if(inner_node->prefix_len){
        const auto [match_len,prefix_key]=inner_node->check_prefix<T>(key,depth,key_len);
        if(match_len>=inner_node->prefix_len){
            depth+=inner_node->prefix_len;
            goto RECURSIVE_INSERT;
        }

        //prefix mismatch, create a new branch
        ArtNode4 *new_node=new ArtNode4();;
        new_node->prefix_len=match_len;
        memcpy(new_node->prefix,inner_node->prefix,std::min(InnerNode::PREFIX_VEC_LEN,match_len));
        *node_ptr=new_node;

        //adjust old node, part of the old node's previous prefix now will be hold by the new node

        new_node->add_child(prefix_key[match_len],node);
        assert(prefix_key[match_len]!='\0');
        inner_node->prefix_len-=match_len+1; //the 1 is for the 1 byte key that differs

        if(inner_node->prefix_len<=InnerNode::PREFIX_VEC_LEN){      
            memmove(inner_node->prefix,prefix_key+match_len+1,std::min(inner_node->prefix_len,InnerNode::PREFIX_VEC_LEN));
        }
        else{
            memcpy(inner_node->prefix,prefix_key+match_len+1,std::min(inner_node->prefix_len,InnerNode::PREFIX_VEC_LEN));
        }

        //make a leaf for the insert 'key'
        ArtNode *leaf=as_leaf(ArtLeaf<T>::make_leaf(key,key_len,val));
        new_node->add_child(key[depth+match_len],leaf);
        return nullptr;
    }

RECURSIVE_INSERT:
    ArtNode::Ptr *child=this->find_child(inner_node,key[depth]);
    if(child){ //recursive search child if current node matches
        return this->insert_impl(*child,child,key,key_len,depth+1,val);
    }

    //if child not exists, assign a leaf as node's child
    ArtNode *leaf=as_leaf(ArtLeaf<T>::make_leaf(key,key_len,val));

    this->add_child(inner_node,node_ptr,key[depth],leaf);
    return nullptr;
}

template<class T>
ArtNode::Ptr* ARTree<T>::find_child(InnerNode *node, const unsigned char key)
{
    assert(!is_leaf(node));
    switch(node->type) {
        case InnerNode::Node4:{
            return static_cast<ArtNode4*>(node)->find_child(key);
        }
        case InnerNode::Node16:{
            return static_cast<ArtNode16*>(node)->find_child(key);
        }
        case InnerNode::Node48:{
            return static_cast<ArtNode48*>(node)->find_child(key);
        }
        case InnerNode::Node256:{
            return static_cast<ArtNode256*>(node)->find_child(key);
        }
    }
}

template<class T>
void ARTree<T>::add_child(InnerNode *node, ArtNode::Ptr *node_ptr, const unsigned char key, ArtNode *child)
{
    switch(node->type) {
        case InnerNode::Node4:{
            ArtNode4 *node4=static_cast<ArtNode4*>(node);
            if(!node4->is_full()){
                node4->add_child(key,child);
            }
            else{
                ArtNode16 *node16=static_cast<ArtNode16*>(node4->expand());
                *node_ptr=node16;
                node16->add_child(key,child);
            }
        }break;

        case InnerNode::Node16:{
            ArtNode16 *node16=static_cast<ArtNode16*>(node);
            if(!node16->is_full()){
                node16->add_child(key,child);
            }
            else{
                ArtNode48 *node48=static_cast<ArtNode48*>(node16->expand());
                *node_ptr=node48;
                node48->add_child(key,child);
            }
        }break;

        case InnerNode::Node48:{
            ArtNode48 *node48=static_cast<ArtNode48*>(node);
            if(!node48->is_full()){
                node48->add_child(key,child);
            }
            else{
                ArtNode256 *node256=static_cast<ArtNode256*>(node48->expand());
                *node_ptr=node256;
                node256->add_child(key,child);
            }
        }break;

        case InnerNode::Node256:{
            ArtNode256 *node256=static_cast<ArtNode256*>(node);
            node256->add_child(key,child);
        }
    }
}

}



#endif // ART_HPP
