#include <gtest/gtest.h>
#include "core/CollectionManager.h"

using namespace testing;
using namespace std;
using namespace Skilo;

TEST(COLLECTION_MANAGER_TEST,CRUD_TEST){
    bool init=true;
    static SkiloConfig conf;
    conf.set_db_dir("/tmp/collection_manager_tst");
    CollectionManager collection_manager(conf);
    std::string schema_str="{\
                            \"name\":\"recipe\",\
                            \"tokenizer\":\"jieba\",\
                            \"auto_suggestion\":{\
                                \"entry_num\":5,\
                                \"min_gram\":1,\
                                \"max_gram\":15\
                            },\
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
                                      \"difficulty\":{\
                                          \"type\":\"integer\",\
                                          \"sort_field\":false\
                                      },\
                                      \"rank\":{\
                                          \"type\":\"integer\",\
                                          \"sort_field\":true\
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
                             \"auto_suggestion\":{\
                                 \"entry_num\":5,\
                                 \"min_gram\":1,\
                                 \"max_gram\":15\
                             },\
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
                                       \"difficulty\":{\
                                           \"type\":\"integer\",\
                                           \"sort_field\":false\
                                       },\
                                       \"rank\":{\
                                           \"type\":\"integer\",\
                                           \"sort_field\":true\
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
         cout<<collection_manager.create_collection(collection_meta)<<endl;
     }

     //collection2
     CollectionMeta collection_meta2(schema_str2);
     string collection_name2=collection_meta2.get_collection_name();
     if(init){
         cout<<collection_manager.create_collection(collection_meta2)<<endl;
     }

     if(init){
         std::ifstream file("dataset/document.json",ios::in|ios::ate);
         size_t size=file.tellg();
         file.seekg(0,ios::beg);
         string str(size,'\0');
         file.read(str.data(),size);
         file.close();

         DocumentBatch doc_batch(collection_name,str);
         std::vector<Document> &docs=doc_batch.get_docs();
         for(Document &d:docs){
             cout<<collection_manager.add_document(collection_name,d)<<endl;
         }

         DocumentBatch doc_batch2(collection_name2,str);
         std::vector<Document> &docs2=doc_batch2.get_docs();
         for(Document &d:docs2){
             cout<<collection_manager.add_document(collection_name2,d)<<endl;
         }
     }

     std::string search_str="{\
                            \"query\": \"镇得住场面\",\
                            \"query by\": [\"recipe_name\",\"context\"]\
                            }";
    Query query("recipe",search_str);
    cout<<collection_manager.search(query)<<endl;

    std::string search_str2="{\
                           \"query\": \"酸菜鱼\",\
                           \"query by\": [\"recipe_name\",\"ingredients.$items.title\"]\
                           }";
    Query query2("recipe2",search_str2);
    cout<<collection_manager.search(query2)<<endl;
    cout<<collection_manager.auto_suggest(collection_name,"镇得住")<<endl;
    cout<<collection_manager.auto_suggest(collection_name2,"酸菜")<<endl;

}
