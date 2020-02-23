# Search

* ###  概览

Skilo目前实现了单词word和词组phrase的匹配搜索，假设对于以下文档(省略了文档中的其他字段):

```json
{
    "id": 683275,
    "recipe_name": "鱼香肉丝",
    "context": "前段时间，小少爷因为在外用餐吃过做鱼香肉丝后特别喜欢，于是回家一个劲的鼓动我，我当然得满足他，于是尝试着做了一次，哪个晓得，这一做就不可收拾了，酸、甜、香、鲜，很开胃,味道实在非常的讨巧！大力推荐给各位，一个绝对镇得住场面的家常菜，主妇必学人气菜式之一。"
    ...
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
{"found":1,"hits":[{"id":683275,recipe_name":"鱼香肉丝","context":"前段时间，...一个绝对镇得住场面的家常菜，主妇必学人气菜式之一。...}],"scores":[4.12]}
```

* ### 搜索流程

Skilo搜索流程分为如下几步：

1.  解析客户端发送的Query请求
2. 	提取查询字符串，对其做分词处理。需要注意的是必须使用和索引文档时相同的分词器，否则会对搜索结果造成负面影响。
3.	初始化本次搜索相关信息，如使用[策略模式](https://www.runoob.com/design-pattern/strategy-pattern.html "Title") 生成Scorer用于给文档评分，以及初始化用于收集命中文档的HitCollector。
4.  从CollectionIndexes中搜索query指定字段索引，将HitContext传递给HitCollector，让HitCollector根据HitContext对文档进行评分并收集结果。
5.  从HitCollector中获取TopK评分文档的seq_id号，从storage_service中读取文档，添加相关命中信息并格式化后返回给客户端。

![](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/search_phases.png)

