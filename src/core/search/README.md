# Search

* ###  概览

Skilo目前实现了单词word和词组phrase的匹配搜索，假设对于以下文档(省略了文档中的其他字段):

```json
{
    "id": 683275,
    "recipe_name": "鱼香肉丝",
    "context": "前段时间，小少爷因为在外用餐吃过做鱼香肉丝后特别喜欢，于是回家一个劲的鼓动我，我当然得满足他，于是尝试着做了一次，哪个晓得，这一做就不可收拾了，酸、甜、香、鲜，很开胃,味道实在非常的讨巧！大力推荐给各位，一个绝对镇得住场面的家常菜，主妇必学人气菜式之一。",
    "...":"..."
}
```

搜索context字段中的短语：“镇得住场面”:

```
{
    "query": "镇得住场面",
    "query by": ["context"]
}
```

结果中可以得到该文档，如果有多个结果会按照score得分由高到低排序：

```json
{"found":1,"hits":[{"id":683275,"recipe_name":"鱼香肉丝","context":"前段时间，...一个绝对镇得住场面的家常菜，主妇必学人气菜式之一。...}],"scores":[4.12]}
```

* ### 搜索流程

Skilo搜索流程分为如下几步：

1.  解析客户端发送的Query请求
2. 	提取查询字符串，对其做分词处理。需要注意的是必须使用和索引文档时相同的分词器，否则会对搜索结果造成负面影响。
3.	初始化本次搜索相关信息，如使用[策略模式](https://www.runoob.com/design-pattern/strategy-pattern.html "Title") 生成Scorer用于给文档评分，以及初始化用于收集命中文档的HitCollector。
4.  从CollectionIndexes中搜索query指定字段索引，将HitContext传递给HitCollector，让HitCollector根据HitContext对文档进行评分并收集结果。
5.  从HitCollector中获取TopK评分文档的seq_id号，从storage_service中读取文档，添加相关命中信息并格式化后返回给客户端。

![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/search_phases.png)

* ### 短语查询

  ​		对于短语”镇得住场面“，可能会被分词为”[镇得住“，”场面“]两个词元，对应的unicode偏移量分别为0和3。若一个文档要包含这个短语，需要同时拥有这两个词元，并且文档中出现的这两个词元的偏移量之差与查询字符串3和0之差相等，否则并不能代表文档中存在这个词组，就比如”咸豆腐脑比甜豆腐脑好吃“与”甜豆腐脑比咸豆腐脑好吃“出现了相同的词元但由于词元偏移量不同意思完全不同。

  ​		因此我们将短语查询分为两部，先寻找出现过所有词元的文档，再看这些文档中的词元是否构成了目标短语。由于倒排列表中文档id是顺序排列的，寻找出现所有词元的文档即变成了k个有序列表求交集问题。这个问题我们有两种思路：
  ##### 倒排列表求交集
  1.  先对所有列表按照**长度递增排序**，然后设置k个cursor；假设第一个列表出现的首元素为a[0]，后k-1个列表指针均向后逐个移动直到各自均找到一个元素大于等于a[0]为止；如果后k-1个列表找到的元素均等于a[0]，则a[0]为公共元素，让第一个列表cursor+1再继续上述循环；如果某个列表找到的元素b[j]大于a[0]，则说明小于b[j]的均非公共元素，此时移动第一个列表的cursor直到找到遇到一个大于等于b[j]的元素再继续上述循环。在上述过程中如果有其中一个列表cursor移动到末尾则结束整个流程。

  ​		例如对于下述倒排列表:

  ​	![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/posting_join.png)

  ​		起始时三个列表cursor均在0位置，第一个列表cursor处元素为5，B列表cursor指针一直移动到第一个大于等于5的元素为5，同样C列表cursor指针一直移动到第一个大于等于5的元素为5，因此5为共同元素。cursor_A移动到下一个位置处的元素为9，cursor_B继续向后移动找到第一个大于等于9的元素为17，它不等于要找的9，说明[9,17]之间不会存在公共元素，因此暂停C列表遍历，让A列表cursor继续移动找到第一个大于等于17的元素，A移动了两格到达了17，如此继续下一轮循环。由此可以看出这种算法**尽可能地让短的列表cursor移动更多的步长**，以此提前遍历到某个列表末尾来终止迭代。设列表长度为m，该方式平均时间复杂度为O(km)
  
  2. 第二种方式是取最短的列表S，对于其他列表二分搜索找S[cursor]是否出现，如果在每个列表中都出现则把S[cursor]加入结果。需要注意的是二分时不需要对整个列表范围内进行二分，只需要对后面随着迭代越来越小的candidate区域二分即可。如果列表长度为m，该方式时间复杂度为O(klogm)至O(kmlogm)。如果列表间长度差距不大，第一种方式会更快。如果倒排列表长度间差距比较大，尤其是存在很短的倒排列表时第二种方式会更快；考虑极端情况第一个列表就1024一个元素，其他列表内容均为1,2,3...1024，那么第一种方式需要遍历全部k×1024元素，第二种方式只需遍历1+10×(k-1)个元素。
  
     因此并不存在哪种方式一定比另一种方式快，我觉得第一种方式平均性能更好。现实情况中倒排列表是压缩存储的，压缩算法提供了二分的接口因此目前我采用了第二种方式，以后随着情况变化倒排列表可能会分块存储在磁盘上，算法可能会进一步调整。
  
  ##### 寻找包含短语的文档
  
  ​	上述已经求得了包含所有词元的文档，现在需要计算文档中是否存在用户搜索的词组以及出现了几次。有了doc_id信息可以从每个词元的PostList中取出该词元在文档中出现过的一系列位置offsets，每个offsets列表也是有序存储的，设词元数量为k那么这样也有了k个有序列表。
  ​	![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/phrase_match.png)
  假设搜索字符串为”我们都是宝宝“，被分词为三个词元[”我们“，"都是"，”宝宝“]，那么三个词元在搜索字符串中的偏移量可以作为每个词元倒排列表中offsets的基数，整体思路和倒排列表求交集的方式一相似，计算词元”我们“所属cursor的元素与它的基数之间的相对偏移量relative_pos，然后线性搜索后续每个词元与它们各自的基数是否相等即可，如果每个词元的cursor都找到了一个位置使它们拥有共同的relative_pos，则此时能构成一个短语。如果至少能构成一个短语，则collect此文档。
  
  ```c++
     /// \brief check whether the doc contains the query phrase and if contains, how many
          /// @example:
          /// query phrase including three term "A","B","C" and term offsets are: 0, 2, 4
          /// a doc's offsets of:
          ///  >  query_term_A -> 11, 26, ...
          ///  >  query_term_B -> 7, 13, 19, 28...
          ///  >  query_term_C -> 15, 21, 30, 100...
          ///
          /// @details: since 11-0=13-2=15-4, we got a phrase match the query phrase
          ///           since 26-0=28-2=30-4, we got another match...
          std::vector<std::pair<uint32_t,const PostingList*>> query_offset_entry; //<query term offset, term postings>
          if(exists_in_all_entry){ //the doc contain all searching terms 
              std::vector<std::pair<uint32_t,std::vector<uint32_t>>> term_cursors; //<posting cur, doc term offsets>
              for(std::pair<uint32_t,const PostingList*> pair:query_offset_entry){
                  std::vector<uint32_t> doc_term_offsets=pair.second->get_doc_term_offsets(lead_doc);
                  term_cursors.emplace_back(0,std::move(doc_term_offsets));
              }
  
              std::vector<uint32_t> &leading_term_offsets=term_cursors[0].second;
              uint32_t leading_qt_offset=query_offset_entry[0].first;
              uint32_t phrase_match_count=0;
              while(term_cursors[0].first<leading_term_offsets.size()){
                  long rel_pos,next_rel_pos; //relative position
                  rel_pos=next_rel_pos=leading_term_offsets[0]-leading_qt_offset; // equivalent to "11-0" in above example
  
                  //move each term's cursor except the leading one to where the relative offset is no smaller than rel_pos
                  for(size_t i=1;i<query_term_count;i++){
                      uint32_t cur_pos=term_cursors[i].first;
                      uint32_t following_qt_offset=query_offset_entry[i].first;
                      std::vector<uint32_t> &term_offs=term_cursors[i].second;
  
                      while(cur_pos<term_offs.size()&&term_offs[cur_pos]-following_qt_offset<rel_pos){ //skip '7' in term_B since 7-2<11-0 until reaching 13
                          cur_pos++;
                      }
                      if(cur_pos==term_offs.size()){ //one of the cursor go to the end
                          goto END_OF_MATCH;
                      }
                      term_cursors[i].first=cur_pos;
                      long term_rel=term_offs[cur_pos]-following_qt_offset;
                      if(term_rel>rel_pos){ //one of the following term's relative offset mismatch
                          next_rel_pos=term_rel;
                          break;
                      }
                  }
  
                  if(next_rel_pos>rel_pos){ //handle mismatch
                      uint32_t leading_cur=term_cursors[0].first;
                      // move leading cursor A to next candidate position
                      while(leading_cur<leading_term_offsets.size()&&leading_term_offsets[leading_cur]-leading_qt_offset<next_rel_pos){
                          leading_cur++;
                      }
                      term_cursors[0].first=leading_cur;
                  }
                  else{ //all term's offset match, move cursor A to the right next position
                      phrase_match_count++;
                      term_cursors[0].first++;
                  }
              }
              // collect this hit
              if(phrase_match_count>0){
                  Search::HitContext context{lead_doc,total_doc_count,&field_path,&candidate_postings};
                  collector.collect(context);
              }
          }
  ```
  
* ### 文档评分与TopK搜集

  Skilo目前采用TF-IDF来计算目标文档每个term的权重，然后计算和phrase_match_count的加权和。TF是该文档中词元出现频率，IDF为逆文档频率因子，计算方式为:
  $$
  \lg{N\over{TF+1}}
  $$
  其中N代表文档集合中总共有多少个文档。IDF考虑的不是文档本身的特征，而是特征单词之间的相对重要性，即衡量不同单词对文档的区分能力。一个词在大部分文档中都没有出现，那么IDF越高，说明这个单词信息含量越多，也就越有价值。例如查询字符串为”搜索引擎技术“，包含”技术“的文档很多，可能有各种各样的技术，但我们要找的不是别的技术而是”搜索引擎“技术，因此”搜索引擎“这个词元含有的信息量更多，被赋予更高的权重。

  由于用户通常不会浏览完所有的搜索结果，因此评分完成后我们取前K个得分的文档返回即可。我们不能简单使用优先队列收集前K个文档，因为用户可能在不同字段上搜索关键字，某个文档在一个字段上的得分可能比另一个字段上的得分要高，因此文档的得分需要动态更新。我们使用小顶堆来使每次插入新文档时能够让其和当前已收集的文档中得分最小值进行比较，如果已经收集了K个结果且新文档得分小于堆顶的文档得分则不加入该文档，否则便添加进来；使用hash表来维护文档id到hit_entry的地址映射，以便更新hit_entry得分。我们提前为K个hit_entry分配好空间，避免每次插入时动态分配内存；堆和hash表中的值都指向分配出来的hit_entry空间，以避免堆调整时hash表也要跟着一起调整。关键代码如下：

```c++
class HitCollector{

public:
    HitCollector(const size_t K,std::unique_ptr<Scorer> scorer)：_K(K),_hits(_K+1),_scorer(std::move(scorer)),_heap_index(0),_min_score_heap(_K+1,nullptr){
    	 for(size_t i=1;i<=_K;i++){
        	_min_score_heap[i]=_hits.data()+i;
    	}
    }
    
    void HitCollector::collect(const HitContext &context){
        uint32_t doc_id=context.doc_seq_id;
        float score=_scorer->get_score(context);
        if(this->num_docs_collected()>=_K&&score<=this->top().score){
            return;
        }

        auto it=_doc_id_map.find(doc_id);
        if(it!=_doc_id_map.end()){
            //update the already exists doc's score(higher) and sift heap
            Hit *hit=it->second;
            if(score>hit->score){
                hit->score=score;
                this->heap_sift_down(hit->heap_index);
            }
        }
        else{
            Hit new_hit{score,doc_id,0};
            if(num_docs_collected()>=_K){
               uint32_t victim_id=this->pop_least_score_doc();
               _doc_id_map.erase(victim_id);
            }
            this->push_new_hit(new_hit);
        }
    }

    void HitCollector::push_new_hit(HitCollector::Hit &hit){
        hit.heap_index=++this->_heap_index;
        _doc_id_map[hit.doc_seq_id]=_min_score_heap[_heap_index];
        *_min_score_heap[_heap_index]=hit;

        size_t cur_index=_heap_index;
        while(cur_index>1){
            size_t parent_index=cur_index/2;
            if(_min_score_heap[parent_index]->score<=_min_score_heap[cur_index]->score){
                break;
            }
            swap_entry(&_min_score_heap[parent_index],&_min_score_heap[cur_index]);
            cur_index=parent_index;
        }
    }
           
    void HitCollector::swap_entry(Hit **hit1,Hit **hit2){
        swap((*hit1)->heap_index,(*hit2)->heap_index);
        swap(*hit1,*hit2);
    }
 
private:
    struct Hit{
        float score;
        uint32_t doc_seq_id;
        size_t heap_index=0;
    };

private:
    const size_t _K;
    std::vector<Hit> _hits; //pre-allocated space for heap entry
    std::unique_ptr<Scorer> _scorer;

    size_t _heap_index;
    ///<--points to _hits-->
    std::vector<Hit*> _min_score_heap;
    std::unordered_map<uint32_t,Hit*> _doc_id_map;
};
```























