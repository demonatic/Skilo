# Skilo Search Engine

###### USTC  软院  2019工程实践Project (Under Construction Now)
Skilo is a simple Search Engine implemented in C++. It provides Restful API to create collection with corresponding schema, add documents to collection and word/phrase search services.


### Documents

***
| 0x01     | 0x02     | 0x03       | 0x04     |
| -------- | -------- | ---------- | -------- |
| [项目概览](./src/README.md) | [索引实现](./src/core/index/README.md) |  [Schema实现](./src/core/schema/README.md)| [查询实现](./src/core/search/README.md) |


### Feature

***

* Simple and easy to use RESTful API

*   Nested schema field support
*   Word/Phrase Search
*   Chinese Support
*   Typo tolerant(TODO)
*   Search hint (TODO) 

### Build

### Basic Usage Example

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
            "tips":{
                "type":"string"
            },
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
    }
  }
  ```

* Add a document to collection

  ```http
  POST /collections/<collection_name>/documents
  ```
  ```json
  {
      "id":1001,
      "recipe_name": "麻婆豆腐",
      "tips": "反正很好吃哦，而且做起来很简单呢",
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
              "position": "4",
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
  
* Query Collection

  ```http
  GET /collections/<collection_name>/documents
  ```

  ```json
  {
      "query": "豆腐",
      "query by": ["recipe_name","ingredients.$items.title"]
  }
  ```

  

### Test

Unit Test  and Integration Testing are based on [GoogleTest](https://github.com/google/googletest "Title") framework 

#### Reference

[Beating hash tables with trees? The ART-ful radix trie](https://www.the-paper-trail.org/post/art-paper-notes/ "Title")

[The Adaptive Radix Tree: ARTful Indexing for Main-Memory Databases](https://db.in.tum.de/~leis/papers/ART.pdf "Title") 

[Frame of Reference and Roaring Bitmaps](https://www.elastic.co/cn/blog/frame-of-reference-and-roaring-bitmaps "Title") 