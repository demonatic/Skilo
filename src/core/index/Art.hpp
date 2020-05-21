#ifndef ART_HPP
#define ART_HPP

#include "ArtNode.hpp"

#include <boost/mpl/insert.hpp>

namespace Art{

template <class T>
class ARTree{
public:
    ARTree():_root(nullptr),_size(0){}
    ~ARTree();

    /// @param key can be non-null-terminate string
    /// @note val's resource will be hold by ARTree
    /// @return nullptr if insert success, otherwise return the already existing value
    T* insert(const char *key,size_t key_len,T *val);

    T* insert(const std::string &key,T *val);

    /// @param key: can be non-null-terminate string
    /// @return nullptr if not find, otherwise return the pointer to the value
    T *find(const char *key,size_t key_len) const;

    T *find(const std::string &key) const;

    void erase(const char *key,size_t key_len);

    /// @brief iterate though all elements over the sub tree designated by given prefix
    /// @param on_elm will be called when iterating reaches a leaf
    /// @param early_termination will be called when key descend to a lower level, return true would stop current node's dfs process
    /// @param on_backtrace will be called when key backtrace to a higher level
    void iterate(const char *prefix,size_t prefix_len,std::function<void(unsigned char *, size_t, T*)> on_elm,
                    std::function<bool(unsigned char)> early_termination=[](unsigned char){return false;},
                        std::function<void(unsigned char)> on_backtrace=[](unsigned char){return;}) const;

    size_t size() const;

private:
    ArtNode::Ptr *find_child(InnerNode *node,const unsigned char child_key) const;
    void add_child(InnerNode *node,ArtNode::Ptr &node_ptr,const unsigned char child_child_key,ArtNode *child);
    void remove_child(InnerNode *node,ArtNode::Ptr &node_ptr,ArtNode::Ptr *child_ptr,unsigned char key);

    T* insert_impl(ArtNode *node,ArtNode::Ptr &node_ptr,const unsigned char *child_key,size_t child_key_len,size_t depth,T *val);
    bool erase_impl(ArtNode *node,ArtNode::Ptr &node_ptr,const unsigned char *child_key,size_t child_key_len,size_t depth);
    void destroy_impl(ArtNode *node);

    void iterate_impl(ArtNode *node,int depth,std::function<void(unsigned char *, size_t, T*)> on_elm,
        std::function<bool(const unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const;

private:
    ArtNode *_root;
    uint64_t _size;
};


template<class T>
ARTree<T>::~ARTree(){
    this->destroy_impl(this->_root);
}

template<class T>
T *ARTree<T>::insert(const char *key, size_t key_len,T *val)
{
    T *existed_val=insert_impl(_root,_root,reinterpret_cast<const unsigned char*>(key),key_len,0,val);
    if(!existed_val)
        _size++;

    return existed_val;
}

template<class T>
T *ARTree<T>::insert(const string &key, T *val)
{
    return this->insert(key.data(),key.length(),val);
}

/// Optimistic Search
/// InnerNode lookups just skip the node's prefix without comparing them.
/// Instead, when a lookup arrives at a leaf its child_key must be compared to the search
/// child_key to ensure that no “wrong turn” was taken
template<class T>
T *ARTree<T>::find(const char *key,size_t key_len) const
{
    ArtNode *node=_root;
    size_t depth=0;
    while(node){
        if(is_leaf(node)){
            ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
            return leaf->key_match(reinterpret_cast<const unsigned char*>(key),key_len)?leaf->data:nullptr;
        }
        InnerNode *inner_node=as_inner_node(node);
        if(inner_node->prefix_len){
            depth+=inner_node->prefix_len;
        }
        if(depth>key_len){
            return nullptr;
        }
        ArtNode::Ptr *child=this->find_child(inner_node,key[depth]);
        node=child?*child:nullptr;
        depth++;
    }
    return nullptr;
}

template<class T>
T *ARTree<T>::find(const string &key) const
{
    return find(key.data(),key.length());
}

template<class T>
void ARTree<T>::erase(const char *key, size_t key_len)
{
    bool ok=this->erase_impl(_root,_root,reinterpret_cast<const unsigned char*>(key),key_len,0);
    if(ok) _size--;
}

template<class T>
void ARTree<T>::iterate(const char *prefix, size_t prefix_len, std::function<void(unsigned char *, size_t, T*)> on_elm,
                        std::function<bool(unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const
{
    size_t depth=0;
    ArtNode *node=_root;

    while(node&&depth<prefix_len){
        if(is_leaf(node)){
            break;
        }
        InnerNode *inner_node=as_inner_node(node);
        for(int i=depth;i<inner_node->prefix_len;i++){
            if(early_termination(inner_node->prefix[i])){
                return;
            }
        }
        depth+=inner_node->prefix_len;
        if(depth<prefix_len){
            ArtNode::Ptr *child=this->find_child(inner_node,prefix[depth]);
            node=child?*child:nullptr;
            if(early_termination(prefix[depth])){
                return;
            }
            depth++;
        }
    }
    iterate_impl(node,depth,on_elm,early_termination,on_backtrace);
}

template<class T>
void ARTree<T>::iterate_impl(ArtNode *node,int depth,std::function<void(unsigned char *, size_t, T*)> on_elm,
                             std::function<bool(unsigned char)> early_termination,std::function<void(unsigned char)> on_backtrace) const
{
    if(!node)
        return;

    if(is_leaf(node)){
        int i;
        ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
        for(i=depth;i<leaf->key_len;i++){
            if(early_termination(leaf->get_key()[i])){
                goto backtrace;
            }
        }
        on_elm(leaf->get_key(),leaf->key_len,leaf->data);
backtrace:
        for(i=i-1;i>=depth;i--){
            on_backtrace(leaf->get_key()[i]);
        }
        return;
    }
    InnerNode *inner_node=as_inner_node(node);
    for(int i=0;i<inner_node->prefix_len;i++){
        if(early_termination(inner_node->prefix[i])){
            for(int j=i-1;j>=0;j--){
                on_backtrace(inner_node->prefix[j]);
            }
            return;
        }
    }
    switch(inner_node->type) {
        case InnerNode::Node4:{
            ArtNode4 *node4=as_node4(inner_node);
            for(int i=0;i<node4->num_children;i++){
                if(!early_termination(node4->keys[i])){
                    iterate_impl(node4->children[i],depth+1,on_elm,early_termination,on_backtrace);
                    on_backtrace(node4->keys[i]);
                }
            }
        }
        break;
        case InnerNode::Node16:{
            ArtNode16 *node16=as_node16(inner_node);
            for(int i=0;i<node16->num_children;i++){
                if(!early_termination(node16->keys[i])){
                    iterate_impl(node16->children[i],depth+1,on_elm,early_termination,on_backtrace);
                    on_backtrace(node16->keys[i]);
                }
            }
        }
        break;
        case InnerNode::Node48:{
            ArtNode48 *node48=as_node48(inner_node);
            for(int i=0;i<256;i++){
                size_t index=node48->child_indexs[i];
                if(index!=0&&!early_termination(i)){
                    iterate_impl(node48->children[index-1],depth+1,on_elm,early_termination,on_backtrace);
                    on_backtrace(i);
                }
            }
        }
        break;
        case InnerNode::Node256:{
            ArtNode256 *node256=as_node256(inner_node);
            for(int i=0;i<256;i++){
                if(node256->children[i]&&!early_termination(i)){
                    iterate_impl(node256->children[i],depth+1,on_elm,early_termination,on_backtrace);
                    on_backtrace(i);
                }
            }
        }
        break;
    }
    for(int i=inner_node->prefix_len-1;i>=0;i--){
        on_backtrace(inner_node->prefix[i]);
    }
}

template<class T>
size_t ARTree<T>::size() const
{
    return _size;
}

template<class T>
T *ARTree<T>::insert_impl(ArtNode *node,ArtNode::Ptr &node_ptr, const unsigned char *key, size_t key_len, size_t depth, T *val)
{
    if(!node){
        node_ptr=store_as_leaf(ArtLeaf<T>::make_leaf(key,key_len,val));
        return nullptr;
    }

    if(is_leaf(node)){
        ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
        if(leaf->key_match(key,key_len)){ //the child_key we are updating already exists
            return leaf->data;
        }

        /// two leaves(origin leaf node and the inserted one) become two children of a node4
        ArtNode4 *new_node=new ArtNode4();

        size_t match_cur; //the longest common prefix length of child_key in the leaf node and 'child_key'
        const unsigned char *leaf_key=leaf->is_key_local()?leaf->key_local:leaf->key;
        for(match_cur=depth;match_cur-depth<InnerNode::PREFIX_VEC_LEN&&key[match_cur]==leaf_key[match_cur];match_cur++){
            new_node->prefix[match_cur-depth]=key[match_cur];
        }
        // the prefix_len field may longer than the actual prefix array
        new_node->prefix_len=match_cur-depth;
        new_node->add_child(leaf_key[match_cur],store_as_leaf(node));

        ArtLeaf<T> *insert_leaf=ArtLeaf<T>::make_leaf(key,key_len,val);
        new_node->add_child(key[match_cur],store_as_leaf(insert_leaf));

        node_ptr=new_node;
        return nullptr;
    }

    InnerNode *inner_node=as_inner_node(node);
    if(inner_node->prefix_len){
        const auto [match_len,prefix_child_key]=inner_node->check_prefix<T>(key,depth,key_len);
        if(match_len>=inner_node->prefix_len){
            depth+=inner_node->prefix_len;
            goto RECURSIVE_INSERT;
        }

        //prefix mismatch, create a new branch
        ArtNode4 *new_node=new ArtNode4();
        new_node->prefix_len=match_len;
        memcpy(new_node->prefix,inner_node->prefix,std::min(InnerNode::PREFIX_VEC_LEN,match_len));
        node_ptr=new_node;

        //adjust old node, part of the old node's previous prefix now will be hold by the new node
        new_node->add_child(prefix_child_key[match_len],node);
        inner_node->prefix_len-=match_len+1; //the 1 is for the 1 byte child_key that differs

        const unsigned char *copy_src=prefix_child_key+match_len+1;
        size_t copy_len=std::min(inner_node->prefix_len,InnerNode::PREFIX_VEC_LEN);
        if(inner_node->prefix_len<=InnerNode::PREFIX_VEC_LEN){
            memmove(inner_node->prefix,copy_src,copy_len);
        }
        else{
            memcpy(inner_node->prefix,copy_src,copy_len);
        }

        //make a leaf for the insert 'child_key'
        ArtNode *leaf=store_as_leaf(ArtLeaf<T>::make_leaf(key,key_len,val));
        new_node->add_child(key[depth+match_len],leaf);
        return nullptr;
    }

RECURSIVE_INSERT:
    ArtNode::Ptr *child=this->find_child(inner_node,key[depth]);
    if(child){ //recursive search child if current node matches
        return this->insert_impl(*child,*child,key,key_len,depth+1,val);
    }

    //if child not exists, assign a leaf as node's child
    ArtLeaf<T> *leaf=ArtLeaf<T>::make_leaf(key,key_len,val);
    this->add_child(inner_node,node_ptr,key[depth],store_as_leaf(leaf));
    return nullptr;
}

template<class T>
bool ARTree<T>::erase_impl(ArtNode *node, ArtNode::Ptr &node_ptr, const unsigned char *child_key, size_t child_key_len, size_t depth)
{
    if(!node)
        return false;

    if(is_leaf(node_ptr)){
        ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
        if(leaf->key_match(child_key,child_key_len)){
            delete leaf;
            node_ptr=nullptr;
            return true;
        }
        return false;
    }

    InnerNode *inner_node=as_inner_node(node);
    depth+=inner_node->prefix_len;
    ArtNode::Ptr *child=this->find_child(inner_node,child_key[depth]);
    if(!child)
        return false;

    // the child node is a leaf, delete this child
    if(is_leaf(*child)){
        ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(*child);
        if(!leaf->key_match(child_key,child_key_len)){
            return false;
        }
        this->remove_child(inner_node,node_ptr,child,child_key[depth]);
        delete leaf;
        return true;
    }
    return erase_impl(*child,*child,child_key,child_key_len,depth+1);
}

template<class T>
void ARTree<T>::destroy_impl(ArtNode *node)
{
    if(!node)
        return;

    if(is_leaf(node)){
        ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
        delete leaf;
    }
    else{
        InnerNode *inner_node=as_inner_node(node);
        switch(inner_node->type){
            case InnerNode::Node4:{
                ArtNode4 *node4=as_node4(inner_node);
                for(int i=0;i<node4->num_children;i++){
                    destroy_impl(node4->children[i]);
                }
                delete node4;
            }
            break;
            case InnerNode::Node16:{
                ArtNode16 *node16=as_node16(inner_node);
                for(int i=0;i<node16->num_children;i++){
                    destroy_impl(node16->children[i]);
                }
                delete node16;
            }
            break;
            case InnerNode::Node48:{
                ArtNode48 *node48=as_node48(inner_node);
                for(int i=0;i<node48->num_children;i++){
                    destroy_impl(node48->children[i]);
                }
                delete node48;
            }
            break;
            case InnerNode::Node256:{
                ArtNode256 *node256=as_node256(inner_node);
                for(int i=0;i<256;i++){
                    if(node256->children[i]){
                        destroy_impl(node256->children[i]);
                    }
                }
                delete node256;
            }
        }
    }
}

template<class T>
void ARTree<T>::remove_child(InnerNode *node, ArtNode::Ptr &node_ptr,ArtNode::Ptr *child_ptr,unsigned char key)
{
    switch(node->type) {
        case InnerNode::Node4:{
            ArtNode4 *node4=as_node4(node);
            node4->remove_child(child_ptr);

            if(node4->num_children==1){ //collapse node with only one child
                ArtNode *child=node4->children[0];
                if(!is_leaf(child)){ //child is inner node, first prepend node's prefix to child inner node
                    InnerNode *child_inner=as_inner_node(child);
                    size_t prefix_prepend_len=std::min(node4->prefix_len+1,InnerNode::PREFIX_VEC_LEN); //1 for the key
                    if(prefix_prepend_len<InnerNode::PREFIX_VEC_LEN){
                       std::memmove(child_inner->prefix+prefix_prepend_len,child_inner->prefix,InnerNode::PREFIX_VEC_LEN-prefix_prepend_len);
                       child_inner->prefix[prefix_prepend_len]=node4->keys[0];
                    }
                    std::memcpy(node4->prefix,child_inner->prefix,prefix_prepend_len-1);
                    child_inner->prefix_len+=node->prefix_len+1;
                }
                //if child is leaf, just delete node4 anyway
                node_ptr=child;
                delete node4;
            }
        }break;

        case InnerNode::Node16:{
            ArtNode16 *node16=as_node16(node);
            node16->remove_child(child_ptr);
            if(node16->can_shrink()){
                ArtNode4 *node4=node16->shrink();
                node_ptr=node4;
            }
        }break;

        case InnerNode::Node48:{
            ArtNode48 *node48=as_node48(node);
            node48->remove_child(key);
            if(node48->can_shrink()){
                ArtNode16 *node16=node48->shrink();
                node_ptr=node16;
            }
        }break;

        case InnerNode::Node256:{
            ArtNode256 *node256=as_node256(node);
            node256->remove_child(key);
            if(node256->can_shrink()){
                ArtNode48 *node48=node256->shrink();
                node_ptr=node48;
            }
        }
    }
}

template<class T>
void ARTree<T>::add_child(InnerNode *node, ArtNode::Ptr &node_ptr, const unsigned char child_key, ArtNode *child)
{
    switch(node->type) {
        case InnerNode::Node4:{
            ArtNode4 *node4=as_node4(node);
            if(!node4->is_full()){
                node4->add_child(child_key,child);
            }
            else{
                ArtNode16 *node16=node4->expand();
                node_ptr=node16;
                node16->add_child(child_key,child);
            }
        }break;

        case InnerNode::Node16:{
            ArtNode16 *node16=as_node16(node);
            if(!node16->is_full()){
                node16->add_child(child_key,child);
            }
            else{
                ArtNode48 *node48=node16->expand();
                node_ptr=node48;
                node48->add_child(child_key,child);
            }
        }break;

        case InnerNode::Node48:{
            ArtNode48 *node48=as_node48(node);
            if(!node48->is_full()){
                node48->add_child(child_key,child);
            }
            else{
                ArtNode256 *node256=node48->expand();
                node_ptr=node256;
                node256->add_child(child_key,child);
            }
        }break;

        case InnerNode::Node256:{
            ArtNode256 *node256=as_node256(node);
            node256->add_child(child_key,child);
        }
    }
}

template<class T>
ArtNode::Ptr* ARTree<T>::find_child(InnerNode *node, const unsigned char child_key) const
{
    switch(node->type) {
        case InnerNode::Node4:{
            return as_node4(node)->find_child(child_key);
        }
        case InnerNode::Node16:{
            return as_node16(node)->find_child(child_key);
        }
        case InnerNode::Node48:{
            return as_node48(node)->find_child(child_key);
        }
        case InnerNode::Node256:{
            return as_node256(node)->find_child(child_key);
        }
    }
    return nullptr;
}


}



#endif // ART_HPP
