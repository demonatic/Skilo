#include <gtest/gtest.h>
#include "../../src/core/CollectionManager.h"

using namespace testing;
using namespace std;
using namespace Skilo;

TEST(COLLECTION_MANAGER_TEST,ADD_DOC_TEST){
    CollectionManager collection_manager("/tmp/skilo_tst");
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
     string collection_name="recipe";
     CollectionMeta collection_meta(schema_str);
     Status create_res=collection_manager.create_collection(collection_name,collection_meta);
     cout<<create_res.description<<endl;
     EXPECT_TRUE(create_res.code==RetCode::CREATED);
     {
         std::ifstream file("/home/demonatic/Projects/Engineering Practice/Skilo/test/document.json",ios::in|ios::ate);
         size_t size=file.tellg();
         file.seekg(0,ios::beg);
         string str(size,'\0');
         file.read(str.data(),size);
         file.close();
         Document document(collection_name,str);
         Status add_res=collection_manager.add_document(collection_name,document);
         cout<<add_res.description<<endl;
         EXPECT_TRUE(add_res.code==RetCode::CREATED);
     }
     std::string search_str="{\
                            \"query\": \"鱼香肉丝\",\
                            \"query by\": [\"recipe_name\",\"context\"]\
                            }";
    Query query("recipe",search_str);
    Status query_res=collection_manager.search(query);
    EXPECT_TRUE(query_res.code==RetCode::OK);
    cout<<query_res.description<<endl;
}