# Search

* ###  概览

Skilo目前实现了单词word和词组phrase的匹配搜索和基于拼音+编辑距离的模糊搜索，假设对于以下文档(省略了文档中的其他字段):

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
  假设搜索字符串为”我们都是宝宝“，被分词为三个词元[”我们“，"都是"，”宝宝“]，那么三个词元在搜索字符串中的偏移量可以作为每个词元倒排列表中offsets的基数，整体思路和倒排列表求交集的方式一相似，计算词元”我们“所属cursor的元素与它的基数之间的相对偏移量relative_pos，然后线性搜索后续每个词元与它们各自的基数是否相等即可，如果每个词元的cursor都找到了一个位置使它们拥有共同的relative_pos，则此时能构成一个短语。如果至少能构成一个短语，则collect此文档。当然offset间匹配实际上无需如此严格，允许相差一定间隔(sloppy)
  
  ```c++
  void InvertIndex::search_field(const std::string &field_path,const std::unordered_map<string,std::vector<uint32_t>> 		&token_to_offsets,const std::vector<size_t> &costs,size_t slop,uint32_t total_doc_count,const std::unordered_map<std::string, SortIndex> *sort_indexes,std::function<void(Search::MatchContext&)> on_match) const
  {
      /// \brief find the conjuction doc_ids of the query terms
      /// @example:
      /// query_term_A -> doc_id_1, doc_id_5, ...
      /// query_term_B -> doc_id_2, doc_id_4, doc_id_5...
      /// query_term_C -> doc_id_5, doc_id_7, doc_id_15, doc_id 29...
      /// conjuction result: doc_id_5 ...
  
      std::vector<const PostingList*> candidate_postings; //for term AND logic, none duplicate
      std::vector<std::pair<uint32_t,const PostingList*>> query_offset_entry; //<query term offset, term postings>, for phrase match
  
      ReaderLockGuard lock_guard(this->_term_posting_lock);
      for(const auto &[query_term,offsets]:token_to_offsets){
          auto *entry=this->get_postinglist(query_term);
          if(!entry) return;
          candidate_postings.push_back(entry);
          for(const uint32_t &off:offsets){
              query_offset_entry.push_back({off,entry});
          }
      }
  
      size_t query_term_count=query_offset_entry.size();
      //sort the posting list based on length in ascending order reduces the number of comparisons
      std::sort(candidate_postings.begin(),candidate_postings.end(),[](const auto &e1,const auto &e2){
          return e1->num_docs()<e2->num_docs();
      });
      //sort query term according to offset in ascending order
      std::sort(query_offset_entry.begin(),query_offset_entry.end(),[](auto &p1,auto &p2){
          return p1.first<p2.first;
      });
  
      uint32_t leading_doc_num=candidate_postings[0]->num_docs();
      std::unique_ptr<uint32_t[]> leading_docs=candidate_postings[0]->get_all_doc_ids();
  
      for(uint32_t leading_cur=0;leading_cur<leading_doc_num;leading_cur++){
          bool exists_in_all_entry=true;
          uint32_t lead_doc=leading_docs[leading_cur];
          for(size_t i=1;i<candidate_postings.size();i++){
              if(!candidate_postings[i]->contain_doc(lead_doc)){ //couldn't find lead_doc in this entey
                  exists_in_all_entry=false;
                  break;
              }
          }
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
  
          if(exists_in_all_entry){
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
                  rel_pos=next_rel_pos=leading_term_offsets[term_cursors[0].first]-leading_qt_offset; // equivalent to "11-0" in above example
  
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
  
                  if(next_rel_pos>rel_pos+slop){ //handle mismatch
                      uint32_t leading_cur=term_cursors[0].first;
                      // move leading cursor A to next candidate position
                      while(leading_cur<leading_term_offsets.size()&&leading_term_offsets[leading_cur]-leading_qt_offset<next_rel_pos){
                          leading_cur++;
                      }
                      term_cursors[0].first=std::max(leading_cur,term_cursors[0].first+1);
                  }
                  else{ //all term's offset match, move cursor A to the right next position
                      phrase_match_count++;
                      term_cursors[0].first++;
                  }
              }
              // collect this hit
              Search::MatchContext context{lead_doc,total_doc_count,phrase_match_count,1,field_path,candidate_postings,costs,token_to_offsets,*sort_indexes};
              on_match(context);
          }
  END_OF_MATCH:
          void(0);
      }
  }
  ```
  
* ### 模糊搜索
  实现上需要分两步，一是实现一算法能衡量两个字符串s和t直接的相似度k，二是找到搜索引擎单词词典中所有与搜索词相似度≤k的词。

衡量两个字符串s和t直接的相似度可以用Levenshtein编辑距离来衡量。两个单词之间的Levenshtein Distance是将一个单词转换为另一个单词所需的单字符编辑（插入、删除或替换）的最小数量。Levenshtein Distance是1965年由苏联数学家Vladimir Levenshtein发明的。Levenshtein Distance也被称为编辑距离（Edit Distance）。

  编辑距离可是使用二维动态规划来求解，对于长为s和t的两个字符串，求解编辑距离的时间复杂度和空间复杂度均为O(st)，过程如图所示：

  ![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/fuzzy1.png)

  寻找搜索引擎单词词典中所有与搜索词相似度≤k的词也有多种方式。蛮力法可以遍历单词词典，对其中所有词与搜索词逐个计算编辑距离，设单词词典有n个词，平均长度为l，搜索词长为m，则时间复杂度为O(n*l*m) ,过高。

  有以下一些方式来降低复杂度：

  1. 使用前缀树增量更新编辑距离二维表

  既然单词词典使用前缀树，那么沿着单词词典构成的前缀树从根到叶子的一条路径增量构建编辑距离动态规划二维表，当表构建完后，整条前缀树路径上的所有单词编辑距离都可以得到。

  并且从根到叶子某条路径上的编辑距离计算可以提前终止，因为执行前述沿前缀树某条路径增量更新编辑距离表的过程时，若某一行最小值大于我们允许的最大编辑距离，则后续以当前单词为前缀的词编辑距离不可能低于最大编辑距离，可以提前终止。由此时间复杂度降低为O(n’*m)，由于提前终止，对于大多数搜索词，n’<<n。

  此外若查询可能用到的最大编辑距离为kmax，则可以给单词词典前缀树每个结点设置一个大小为kmax位的位图，记录从该结点开始到后续所有单词的长度有哪些，如果搜索词长度为l，允许编辑距离为k，则如果[l-k,l+k]范围的位图内都没有设置为1，则说明这条路径上不存在我们想要编辑距离范围内的词，此时也可以提前终止。

  ![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/fuzzy2.png)

  2. BK-tree

  BK树使用Levenshtein编辑距离所满足的三角不等式性质：

  Levenstein(A,B)+Levenstein(A,C)≥Levenstein(B,C)

  Levenstein(A,B)−Levenstein(A,C)≤Levenstein(B,C).

  在索引期间Levenshtein(root node,child node)被提前计算好。在查询时计算Levenshtein(input ,root node)，前述三角不等式被用于过滤，来递归寻找提前计算的编辑距离在[Levenstein(input ,root node)-dmax, Levenstein(input ,root node)+dmax]范围内的子节点。
详情见：[The BK-Tree – A Data Structure for Spell Checking](https://nullwords.wordpress.com/2013/03/13/the-bk-tree-a-data-structure-for-spell-checking/) 

  3. SymSpell Algorithm

  主要思路为索引时对插入单词词典的词进行deletion操作产生一系列删除一定字符后的词，和原词一并插入单词词典；对于搜索词也同样做deletion操作，用产生的词和原词进行搜索。对于一个长度为n的词，字典大小为a,编辑距离为1，最多有n次删除，一共搜索n个词。
  详情见：[1000x Faster Spelling Correction algorithm](https://medium.com/@wolfgarbe/1000x-faster-spelling-correction-algorithm-2012-8701fcd87a5f) 

* ### 搜索提示
  实现该功能的主要数据结构为两个哈希表。第一个哈希表存完整的串以及它的出现频率，第二个哈希表存前缀串到以该前缀串开头的按出现频率排序由高到低排序的大小为k的完整热串列表。
  
  当一个串s被索引时，先访问第一个hash表将其频率+1，假设出现频率为m，然后使用edge-ngram算法进行切分，生成n个前缀串，如串‘鸡蛋饼’被切分为[‘鸡’，’鸡蛋‘，’鸡蛋饼‘]三个前缀串，对这三个前缀串查询第二个哈希表，如果大小为k的热串列表中已经有串s，则将其在列表中的频率+1，并调整到合适的位置使列表保持有序；如果s不在列表中，且其频率m>列表中最小频率热串的频率，则将该最小频率热串替换为s。
  
	![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/search_suggest.png)
	![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Auto%20Suggestion.png)
	设每个串使用edge-lgram进行前缀切分，每次搜索提示k个热词/串，则算法更新时间复杂度为O(k*l)，查询给定前缀开头的k个热词/串时间复杂度为O(k)
	
* ### 文档评分与TopK搜集

  Skilo目前支持以下几种评分和排序方式：TF-IDF, BM25,按一个或多个数值字段递增递减排序。
  对于TF-IDF方式而言：TF是该文档中词元出现频率，IDF为逆文档频率因子，计算方式为:
  
  ```mathematica
    tdidf=tf*log(N/k)
  ```

  其中N代表文档集合中总共有多少个文档,k代表出现过该词元的文档数目。IDF考虑的不是文档本  身的特征，而是特征单词之间的相对重要性，即衡量不同单词对文档的区分能力。一个词在大部分  文档中都没有出现，那么IDF越高，说明这个单词信息含量越多，也就越有价值。例如查询字符串  为”搜索引擎技术“，包含”技术“的文档很多，可能有各种各样的技术，但我们要找的不是别的技  术而是”搜索引擎“技术，因此”搜索引擎“这个词元含有的信息量更多，被赋予更高的权重。

  BM25方式与IF-IDF思路有不少相似之处，但是更为复杂：

  ![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/bm25.png)

  该公式分为三个部分，第一部分为外部公式，表明对于包含多个词项的查询Q，每个词项对整个查询得分的贡献，如对于查询＂中国人说中国话＂，＂中国＂出现了两次qtf=2，说明希望查询到的文档和＂中国＂应该更相关，该词项权重应该更高。第二部分类似于IF-IDF模型中的TF项，某个词项在文档中出现次数越多则该文档越重要。为了抑制长文档词项较多从而词频天然较高的倾向，因此使用文档长度除以平均文档长度进行归一化。第三部分类似于IDF项，对于一些含有比较稀有的查询词的文档给予更高的评分。

  上述每种评分方式都是一种类型的Scorer, 一次查询可能包含多种评分标准，如可以先按照与查询词匹配度排序，匹配度相同时再按照某个数值字段递增排序。由此实现一个DocRanker来对每个文档依次按照不同评分标准评分。

  ```c++
  //根据本次查询需要的评分方式构造一个DocRanker
  class DocRanker
  {
  public:
      DocRanker();
  
      void add_scorer(std::unique_ptr<Scorer> scorer);
      // 对于每个candidate文档调用rank方法，依次调用不同类型的Scorer对文档进行评分
      // 得到该文档在每个评分标准上的分值
      std::vector<number_t> rank(const HitContext &ctx) const{
            std::vector<number_t> rank_scores;
            for(size_t i=0;i<_scorer_num;i++){
                number_t score=_scorers[i]->get_score(ctx);
                rank_scores.push_back(score);
            }
            return rank_scores;
      }
  
      inline static constexpr size_t MaxRankCriteriaCount=4;
  private:
      size_t _scorer_num;
      //多种评分标准的评分器，按优先级依次排列
      std::array<std::unique_ptr<Scorer>,MaxRankCriteriaCount> _scorers;
  };
  ```

  由于用户通常不会浏览完所有的搜索结果，因此评分完成后我们取前K个得分的文档返回即可。我们使用HitCollector类收集top K高评分的文档，它包含一个DocRanker来对文档进行评分。我们不能简单使用优先队列收集前K个文档，因为用户可能在不同字段上搜索关键字，某个文档在一个字段上的得分(如查询匹配度)可能比另一个字段上的得分更高，因此文档的得分需要动态更新。我们使用小顶堆来使每次插入新文档时能够让其和当前已收集的文档中得分最小值进行比较，如果已经收集了K个结果且新文档得分小于堆顶的文档得分则不加入该文档，否则便添加进来；使用hash表来维护文档id到hit_entry的地址映射，以便更新hit_entry得分。我们提前为K个hit_entry分配好空间，避免每次插入时动态分配内存；堆和hash表中的值都指向分配出来的hit_entry空间，以避免堆调整时hash表也要跟着一起调整。关键代码如下：

```c++
class HitCollector{

public:
    HitCollector(const size_t K,DocRanker &ranker);
    
    //收集查询到的candidate文档，进行评分并判断是否加入结果集
    void HitCollector::collect(const HitContext &context){
        uint32_t doc_id=context.doc_seq_id;
        std::vector<number_t> scores=_ranker.rank(context); //计算当前candidate评分
            
        //结果集已包含K个文档且该文档分数比当前结果集中任何文档分数都小
        if(this->num_docs_collected()>=_K&&scores<=this->top().rank_scores){
            return;
        }

        auto it=_doc_id_map.find(doc_id);
        if(it!=_doc_id_map.end()){
            //update the already exists doc's score(higher) and sift heap
            Hit *hit=it->second;
            if(scores>hit->rank_scores){
                hit->rank_scores=std::move(scores);
                heap_sift_down(hit->heap_index);
            }
        }
        else{
            Hit new_hit{doc_id,0,scores};
            if(num_docs_collected()>=_K){ //替换当前堆中分数最小的文档
            uint32_t victim_id=this->pop_least_score_doc();
            _doc_id_map.erase(victim_id);
        }
        this->push_new_hit(new_hit);
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
        uint32_t doc_seq_id;
        size_t heap_index=0;
        std::vector<number_t> rank_scores;

        bool operator<(Hit &other){
            return this->rank_scores<other.rank_scores;
        }
    };

private:
    const size_t _K;
    std::vector<Hit> _hits; //pre-allocated k space for heap entry
    DocRanker _ranker;　//文档评分器

    size_t _heap_index;
    ///<--points to _hits-->
    std::vector<Hit*> _min_score_heap;
    std::unordered_map<uint32_t,Hit*> _doc_id_map;
};
```























