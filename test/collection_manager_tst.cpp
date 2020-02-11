#include <gtest/gtest.h>
#include "../../src/core/CollectionManager.h"

using namespace testing;
using namespace std;
using namespace Skilo;

TEST(COLLECTION_MANAGER_TEST,ADD_DOC_TEST){
    bool init=false;
    CollectionManager collection_manager("/tmp/skilo3");
    std::string schema_str="{\
                            \"name\":\"recipe\",\
                            \"tokenizer\":\"jieba\",\
                            \"schema\":{\
                                \"type\":\"object\",\
                                \"$fields\": {\
                                    \"keyword\":{\
                                        \"type\":\"string\",\
                                        \"index\":true\
                                      },\
                                      \"recipe_name\":{\
                                          \"type\":\"string\",\
                                          \"index\":true\
                                      },\
                                      \"context\":{\
                                          \"type\":\"string\",\
                                          \"index\":true\
                                      },\
                                      \"tips\":{\
                                          \"type\":\"string\"\
                                      },\
                                      \"ingredients\": {\
                                          \"type\": \"array\",\
                                          \"$items\": {\
                                              \"type\": \"object\",\
                                              \"$fields\":{\
                                                   \"note\": {\"type\": \"string\"},\
                                                   \"title\": {\"type\": \"string\", \"index\":true}\
                                              }\
                                          }\
                                      },\
                                      \"steps\": {\
                                          \"type\": \"array\",\
                                          \"$items\": {\
                                              \"type\": \"object\",\
                                              \"$fields\":{\
                                                   \"position\": {\"type\": \"string\"},\
                                                   \"content\": {\"type\": \"string\"},\
                                                   \"thumb\": {\"type\": \"string\"},\
                                                   \"image\": {\"type\": \"string\"}\
                                              }\
                                          }\
                                      }\
                                  }\
                                }\
                             }";

     std::string schema_str2="{\
                             \"name\":\"recipe2\",\
                             \"tokenizer\":\"jieba\",\
                             \"schema\":{\
                                 \"type\":\"object\",\
                                 \"$fields\": {\
                                     \"keyword\":{\
                                         \"type\":\"string\",\
                                         \"index\":true\
                                       },\
                                       \"recipe_name\":{\
                                           \"type\":\"string\",\
                                           \"index\":true\
                                       },\
                                       \"context\":{\
                                           \"type\":\"string\",\
                                           \"index\":true\
                                       },\
                                       \"tips\":{\
                                           \"type\":\"string\"\
                                       },\
                                       \"ingredients\": {\
                                           \"type\": \"array\",\
                                           \"$items\": {\
                                               \"type\": \"object\",\
                                               \"$fields\":{\
                                                    \"note\": {\"type\": \"string\"},\
                                                    \"title\": {\"type\": \"string\", \"index\":true}\
                                               }\
                                           }\
                                       },\
                                       \"steps\": {\
                                           \"type\": \"array\",\
                                           \"$items\": {\
                                               \"type\": \"object\",\
                                               \"$fields\":{\
                                                    \"position\": {\"type\": \"string\"},\
                                                    \"content\": {\"type\": \"string\"},\
                                                    \"thumb\": {\"type\": \"string\"},\
                                                    \"image\": {\"type\": \"string\"}\
                                               }\
                                           }\
                                       }\
                                   }\
                                 }\
                              }";

     //collection1

     CollectionMeta collection_meta(schema_str);
     string collection_name=collection_meta.get_collection_name();
     if(init){
         Status create_res=collection_manager.create_collection(collection_meta);
         cout<<create_res.description<<endl;
         EXPECT_TRUE(create_res.code==RetCode::CREATED);
     }


     //collection2
     CollectionMeta collection_meta2(schema_str2);
     string collection_name2=collection_meta2.get_collection_name();
     if(init){
         Status create_res2=collection_manager.create_collection(collection_meta2);
         cout<<create_res2.description<<endl;
         EXPECT_TRUE(create_res2.code==RetCode::CREATED);
     }

     if(init){
         std::ifstream file("/home/demonatic/Projects/Engineering Practice/Skilo/test/document.json",ios::in|ios::ate);
         size_t size=file.tellg();
         file.seekg(0,ios::beg);
         string str(size,'\0');
         file.read(str.data(),size);
         file.close();

         DocumentBatch doc_batch(collection_name,str);
         std::vector<Document> &docs=doc_batch.get_docs();
         for(Document &d:docs){
             Status add_res=collection_manager.add_document(collection_name,d);
             cout<<add_res.description<<endl;
             EXPECT_TRUE(add_res.code==RetCode::CREATED);

         }

         DocumentBatch doc_batch2(collection_name2,str);
         std::vector<Document> &docs2=doc_batch2.get_docs();
         for(Document &d:docs2){
             Status add_res2=collection_manager.add_document(collection_name2,d);
             cout<<add_res2.description<<endl;
             EXPECT_TRUE(add_res2.code==RetCode::CREATED);
         }
     }

     std::string search_str="{\
                            \"query\": \"镇得住场面\",\
                            \"query by\": [\"recipe_name\",\"context\"]\
                            }";
    Query query("recipe",search_str);
    Status query_res=collection_manager.search(query);
    EXPECT_TRUE(query_res.code==RetCode::OK);
    cout<<query_res.description<<endl;

    std::string search_str2="{\
                           \"query\": \"酸菜鱼\",\
                           \"query by\": [\"recipe_name\",\"context\"]\
                           }";
    Query query2("recipe2",search_str2);
    Status query_res2=collection_manager.search(query2);
    EXPECT_TRUE(query_res2.code==RetCode::OK);
    cout<<query_res2.description<<endl;
}
