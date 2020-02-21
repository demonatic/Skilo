# Index实现

* #### 概述

搜索引擎通常包含两部分索引：倒排索引和正向索引。

倒排索引维护单词到文档id的映射，它是单词文档矩阵的一种具体实现方式，反应了文档和单词之间所具有的一种包含关系。

<img src="https://github.com/demonatic/Image-Hosting/blob/master/Skilo/word_doc_matrix.jpg"  />

​	单词文档矩阵除了倒排索引外也可用签名文件、后缀树等来实现，但大量实践表明倒排索引是单词到文档映射关系的最佳实现方式。

​	倒排索引主要由两部分组成：一是单词字典Dictionary，用于存储文档内容经分词后得到的词项Term；二是倒排列表Postings，它记录每一个词项所对应的文档信息列表。在Postings中文档id号为必须项，此外通常还记录每个文档中出现该词项的频率(Term Frequency)，以及词项在文档中的出现位置offset。

![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/postings.png)

* #### 单词词典实现

​	单词词典可能包含几十万甚至上百万个词项，能够快速定位某个单词，直接影响搜索时的速度，所以在选择词典数据结构实现时，它需要拥有高效的查找性能；而对于占用空间，由于现今内存很大，且词典大小并不会和文档数成正比，通常能全部放入内存，因此占用空间是次要考虑的一方面。

​	首先我考虑到使用哈希表作为字典存储结构，它拥有O(1)查找性能，查找词项速度非常快，但拿它作词典索引有如下缺点：hash表把key随机散步到内存空间里，因此它只能进行点查询，此外hash表在overflow时会面临耗时的O(n)复杂度rehash操作。

​	使用基于Key Comparison的树结构如二叉搜索树也是一种选择，它们提供了O(logn)复杂度的查找性能，且数据是有序的，但对于现代CPU来说key的比较通常结果并不能很好地被预测，因此容易造成CPU指令流水线断流，从而造成额外的开销。因此对它们操作时间复杂度上常数会比较大。

​	第三类选择是使用基于key本身的digital表示的数据结构，如trie树/radix树等。这类结构每次查找时间复杂度为O(k)，其中k为key的长度，而与单词词典中的词项个数无关，如果一个节点存储s比特，则比较次数为k/s；而对于二叉搜索树比较次数为log<sub>2</sub>(N)，当N>2<sup>k/s</sup>时，前缀树比较次数即比二叉搜索树小，对于64比特key和s=8bit时N仅为256。但对于传统的Trie树实现而言，由于我们需要存储Unicode字符集，每个节点需要存储256个孩子节点指针，空间利用率过于低下并且最致命的是能放入L1 cache的节点数将变得很少。

```c++
  struct TrieNode {
        TrieNode* children[256]; //not cache friendly
  }
```

​	对于传统的Trie树有许多更好的实现方式，如DoubleArrayTrie(DAT)树，但Skilo采用了Adative Radix Tree(ART)树，来自论文[The Adaptive Radix Tree: ARTful Indexing for Main-Memory Databases](https://db.in.tum.de/~leis/papers/ART.pdf "Title") ，它最大的特性是节点大小可根据实际孩子个数动态可变，同时使用路径压缩(Path Compression)和惰性展开(Lazy Expansion)机制来减小树高。

![image-20200221223648059](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/ART.png)

​	ART树有四种不同大小的Inner节点，分别对应至多有4, 16, 48, 256种孩子的情况。**逻辑上**它的节点等价于：

```c++
union ArtNode {
    Node4* n4;
    Node16* n16;
    Node48* n48;
    Node256* n256;
}
```

​	对于拥有至多4个孩子的Inner节点，只需将键和child指针按对应关系存储在大小为4的数组中，查找时顺序遍历即可。假设指针为8字节，Node4最多占36字节，可以放入单个cache行内。因此对于十分稀疏的Inner节点Node4十分高效。

```c++
struct ArtNode4:InnerNode{
	ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode::Ptr *child=nullptr;
        for(int i=0;i<num_children;i++){
            if(keys[i]==key){
                child=&children[i];
            }
        }
        return child;
    }
	uint8_t num_children;
    unsigned char keys[4];
    ArtNode *children[4];
}
```

​	对于拥有5-16个孩子的Inner节点，ArtNode16的结构和ArtNode4一样：

```c++
struct ArtNode16:InnerNode{
 ArtNode::Ptr* find_child(const unsigned char key){
        int match_result= 	_mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(key),_mm_loadu_si128((__m128i *)keys)));
         // build a mask to select only the first _num_children values from the comparison
        int mask=(1<<num_children)-1;
        int bit_field=match_result&mask;
         // find the index of the first '1' in the bitfield by counting the tailing zeros.
        int index=__builtin_ctz(bit_field);
        return bit_field!=0?&children[index]:nullptr;
    }
	uint8_t num_children;
    unsigned char keys[16];
    ArtNode *children[16];
}
```

​	在查找时可以使用SIMD指令并行比较16个位置，即_mm_set_epi8将参数key拷贝16份，与节点中已经存储的16个key进行比较，对于多余的结果根据孩子个数mask掉，1bit的位置即为16个key中命中的位置。性能相比二分查找更优。

​	对于拥有17-48个孩子的Inner节点，ArtNode48再采用顺序搜索效率将变低，因此在所有可能的key取值数组child_indexs[256]中存储每个key所对应的，在孩子指针数组ArtNode *children[48]中的下标。如果child_indexs[key]为0说明该key没有孩子，否则孩子指针为children[child_indexs[key]-1]所对应值。

```c++
struct ArtNode48:InnerNode{
	ArtNode::Ptr* find_child(const unsigned char key){
        unsigned char index=child_indexs[key];
        return index?&children[index-1]:nullptr;
    }
	uint8_t num_children;
    unsigned char child_indexs[256]; //index=k represents at children[k-1]
    ArtNode *children[48];
}
```

​	使用这种方式只需存储48个指针，相比为256个key可能值存储256个指针占用空间更小。

​	当Inner节点孩子个数大于48时，使用和传统Trie树一样的节点，但由于此时至少有49个孩子，浪费相比只用TrieNode小不少。

```c++
struct ArtNode256:InnerNode{
	ArtNode::Ptr* find_child(const unsigned char key){
        ArtNode *child=children[key];
        return child!=nullptr?&children[key]:nullptr;
    }
    uint16_t num_children;
    ArtNode *children[256];
}
```

​	上述四种类型的Inner Node用于存储Path，ART树中所有value存储于Leaf中，对于以'\0'结尾的字符串没有一个字符串会是另一个字符串的前缀，当然没有任何问题。论文中介绍了三种存储value的方式，Single-vale leaves, Multi-value leaves以及Combined pointer/value slots; Skilo实现时采用了第一种方式，即value以一种单独的LeafNode类型存储起来，InnerNode孩子指针根据最低bit位是否为1来区分InnerNode和LeafNode。

​	ART树使用lazy compression和path expansion策略来降低树高，这是两种相对应的策略。对于每个节点都只有一个孩子的路径，可以把它们压缩到同一个节点里，将路径上的key值存储到一个prefix域里。如原本"F","O","O"各自占一个节点，现在变成一个节点里存储"FOO"前缀。当新插入一个key “FOM”时，会发生path expansion，此时将"FOO"与"FOM"共同前缀"FO"进行压缩存储，后面"O"和"M"会产生新的分叉。

​	![image-20200221231220172](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/art_lazy_expansion.png)

​	Skilo在实现路径压缩时使用固定大小的栈空间来存储路径共同前缀prefix，对于超出的部分将不存储，只记录共同前缀的长度，以此避免堆内存分配和间接寻址。

```c++
struct InnerNode:public ArtNode
{
    enum NodeType:uint8_t{
        Node4,
        Node16,
        Node48,
        Node256
    }; 
    NodeType type;
    uint32_t prefix_len; //0-n, may larger than PREFIX_VEC_LEN
    static constexpr uint32_t PREFIX_VEC_LEN=10;
    unsigned char prefix[PREFIX_VEC_LEN];
};
```

​	对于插入来说，如果实际prefix_len超出了prefix实际存储长度PREFIX_VEC_LEN，并且路径展开产生新分支处的位置同样也超过了PREFIX_VEC_LEN，那么此时需要寻找下层的某个叶节点根据其中存储的完整的key来获取实际的prefix内容，然而这种情况通常很少发生。

​	对于查询来说，Skilo采取乐观搜索策略，只不断根据InnerNode的prefix_len域获得引起节点分叉的key对应字节，用其来快速导向下层的candidate节点，当一直往下到达叶节点时再一次性比较叶节点存储的key和查询的key是否一致来防止误匹配发生。由此，查询的代码可以很简洁高效:

```c++
template<class T>
T *ARTree<T>::find(const char *key,size_t key_len) const
{
    ArtNode *node=_root;
    size_t depth=0;
    while(node){
        if(is_leaf(node)){
            ArtLeaf<T> *leaf=ArtLeaf<T>::as_leaf_node(node);
            //一次比较待查询key和叶节点key是否相等
            return leaf->key_match(key,key_len)?leaf->data:nullptr;
        }
        InnerNode *inner_node=as_inner_node(node);
        depth+=inner_node->prefix_len; //找到引起分支的key字节(key[depth])
        ArtNode::Ptr *child=this->find_child(inner_node,key[depth]); //导向目标child

        node=child?*child:nullptr;
        depth++;
    }
    return nullptr;
}
```

​	

Skilo中正向索引通常维护文档id到文档具体内容的映射，在Skilo中这一部分主要由KV存储引擎帮助我们完成。RocksDB ...

