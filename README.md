# Skilo Search Engine

###### USTC  软院  2019工程实践Project (Under Construction Now)
Skilo is a simple Search Engine implemented in C++ on Linux. It provides Restful API to create collection with corresponding schema, add documents to collection and word/phrase search services.


## Documents
| 0x01     | 0x02     | 0x03       | 0x04     |
| -------- | -------- | ---------- | -------- |
| [项目概览](./src/README.md) | [索引实现](./src/core/index/README.md) |  [Schema实现](./src/core/schema/README.md)| [查询实现](./src/core/search/README.md) |


## Feature

* Simple and easy to use RESTful API
*   Nested schema field support
*   Word Search/ Phrase Search / Fuzzy search & Typo tolerant
*   Query by/Sort by
*   Chinese Support
*   Auto Suggestion

## Build & Run

```bash
git clone https://github.com/demonatic/Skilo --recursive
cd ./Skilo
cmake .  #build dependencies will take a while
make
make install
Skilo
```

edit the file "/etc/skilo/skilo.conf" in project root directory to change database/log directory, listen port and etc.

## Usage Example

* Create a collection

  ```http
  POST /collections
  ```

  ```json
  {
    "name":"recipe",
    "tokenizer":"jieba",
    "schema":{
        "type":"object",
        "$fields": {
            "recipe_name":{
                "type":"string",
                "index":true
            },
            "difficulty":{"type":"integer"},
            "rank":{"type":"integer"},
            "tips":{"type":"string"},
            "ingredients": {
                "type": "array",
                "$items": {
                    "type": "object",
                    "$fields":{
                        "note": {"type": "string"},
                        "title": {"type": "string", "index":true}
                    }
                }
            },
            "steps": {
                "type": "array",
                "$items": {
                    "type": "object",
                    "$fields":{
                        "content": {"type": "string"},
                        "image": {"type": "string"}
                    }
                }
            }
        }
    },
    "auto_suggestion":{
        "entry_num":5,
        "min_gram":2,
        "max_gram":15
    }
  }
  ```

* Add  document(s) to collection

  ```http
  POST /collections/<collection_name>
  ```
  add single document:
  ```json
  {
      "id":1001,
      "recipe_name": "麻婆豆腐",
      "tips": "反正很好吃哦，而且做起来很简单呢",
      "difficulty":1,
      "rank":4,
      "ingredients": [{
          "note": "500克",
          "title": "豆腐"
      }, {
          "note": "150克",
          "title": "肉末"
      }],
      "steps": [{
              "content": "豆腐切一厘米见方的小块。",
              "image": "/recipe/image/1001/1.jpg"
          },{
              "content": "花椒和麻椒冷油下锅，慢火2-3分钟爆出香味后捞出扔掉。",
              "image": "/recipe/image/1001/2.jpg"
          },{
              "content": "锅里底油放入蒜末和郫县豆瓣酱小火翻炒1-2分钟。",
              "image": "/recipe/image/1001/3.jpg"
          },{
              "content": "然后放入肉末翻炒至熟,炒熟的肉末加入一小碗半开水煮2-3分钟。",
              "image": "/recipe/image/1001/4.jpg"
          },{
              "content": "然后加入豆腐块，不要用铲子翻板，轻轻的将豆腐推开即可，在煮4-5分钟，让豆腐完全入味。",
              "image": "/recipe/image/1001/5.jpg"
          },{
              "content": "出锅前加入少许淀粉水，让汤汁更加浓稠。",
              "image": "/recipe/image/1001/6.jpg"
          }
      ]
  }
  ```
  add batch:
  
  ```
  {
      "docs":[
          <doc1>,
          <doc2>,
          <doc3>
      ]
  }
  ```
  
* Query Collection

  ```http
  GET /collections/<collection_name>/documents
  ```
  in case some client doesn't support GET with body, also:
  ```http
  POST /collections/<collection_name>/documents
  ```

  ```json
  {
      "query": "豆腐",
      "query by": ["recipe_name","ingredients.$items.title"],
      "boost": [2.5,1],
      "sort by":["difficulty:asc","rank:desc"]
  }
  ```

* Query Result

  ```
  {
    "found": 2,
    "hits": [
      {
          "id": "1001",
          "recipe_name": "麻婆豆腐",
          "difficulty":1,
          "rank":4,
          ....
      },
      {
          "id": "1002",
          "recipe_name": "麻婆豆腐",
          "difficulty":2,
          "rank":3,
          ....
      }
    ],
    "scores":[14.86,5.32],
    "took secs": 0.000732,
  }
  ```

* Auto Suggestion

  List top K hot queries start with given prefix <query_prefix>
  
  ```http
  GET /collections/<collection_name>/auto_suggestion?q=<query_prefix>
  ```
  
  e.g. list hot query suggestions start with "红烧" :

  ```json
  {
    "suggestions": [
      "红烧肉",
      "红烧狮子头",
      "红烧带鱼"
    ]
  }
  ```
  
* Overall Summary

  show all collections brief information

  ```http
  GET /collections/
  ```

  we get:

  ```json
  {
    "collections": [
      {"name":"recipe","created at":"Mon Oct 2 00:59:08 2019","doc num":84231},
      {"name":"order","created at":"Thu Jun 18 15:48:19 2020","doc num":5652}
    ]
  }
  ```
  
* Collection Summary

  show brief information(collection name, schema, tokenizer name, created_time, doc num...) about given collection

  ```http
  GET /collections/<collection_name>
  ```

## Test

Unit Test  and Integration Testing are based on [GoogleTest](https://github.com/google/googletest "Title") framework 

## Reference

[Beating hash tables with trees? The ART-ful radix trie](https://www.the-paper-trail.org/post/art-paper-notes/ "Title")

[The Adaptive Radix Tree: ARTful Indexing for Main-Memory Databases](https://db.in.tum.de/~leis/papers/ART.pdf "Title") 

[Frame of Reference and Roaring Bitmaps](https://www.elastic.co/cn/blog/frame-of-reference-and-roaring-bitmaps "Title") 

[现代信息检索](https://blog.idejie.com/2018/11/25/ir-review/) 

[TypeSense Guide](https://typesense.org/docs/0.11.1/guide/ "Title") 

[ELASTICSEARCH 搜索的评分机制](https://www.cnblogs.com/hoojjack/p/8261075.html "Title") 

[Lucene系列（10）——相似度评分机制浅析（终篇）](
https://niyanchun.com/lucene-learning-10.html"Title") 

[Elasticsearch权威指南（中文版）](https://es.xiaoleilu.com/052_Mapping_Analysis/35_Inverted_index.html"Title") 

[Autosuggest Retrieval Data Structures & Algorithms](https://medium.com/related-works-inc/autosuggest-retrieval-data-structures-algorithms-3a902c74ffc8) 


