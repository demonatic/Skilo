# 项目概述

Skilo 大体分为如下几个关键模块：

* Core

  ​	Collection Manager: 用于管理所有Collection，维护全局meta信息，如Collection数目和Collection ID号等，并提供增删查改接口给上层。启动系统时引导所有Collection并行加载。

  ​	Collection: 维护属于自己的meta信息，包括Schema、Collection名称、分词器等。根据Schema构建索引项，索引该集合中所有文档中的指定Field。提供具体的查询服务。

* Storage

  ​	Storage Service封装了搜索引擎所需要的存储服务。包括：提供单个文档的增删以及查询、批量scan文档、读写全局meta信息、读写Collection meta信息等。

  ​	Storage Engine使用嵌入式Key-Value引擎RocksDB，并封装了一层PUT、GET、Scan、Batch Write接口。
  
* Network

  ​	我编写了一个服务端的HTTP库[Rinx](https://github.com/demonatic/Rinx "Title") 来让Skilo能够比较方便地对外提供HTTP服务。Skilo使用RESTful风格的API, 使用Json格式与客户端交换数据。

Skilo 整体架构图如下所示：
<img src="[]([https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Skilo%20Architecture.png](https://github.com/demonatic/Image-Hosting/blob/master/Skilo/Skilo Architecture.png))" style="zoom: 50%;" />