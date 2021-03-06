#include <gtest/gtest.h>
#include "core/CollectionManager.h"
#include "utility/Util.h"
#include "g3log/g3log.hpp"
#include "g3log/filesink.hpp"
#include "g3log/logworker.hpp"
#include "utility/LogSink.h"
#include <memory>

using namespace testing;
using namespace std;
using namespace Skilo;

TEST(COLLECTION_MANAGER_TEST,CRUD_TEST){
    bool init=true;
    static SkiloConfig conf;
    std::unique_ptr<g3::LogWorker> log_worker;
    std::unique_ptr<g3::FileSinkHandle> log_file_handle;
    log_worker = g3::LogWorker::createLogWorker();

    log_worker->addSink(std::make_unique<CustomLogSink>(),&CustomLogSink::ReceiveLogMessage);
    g3::initializeLogging(log_worker.get());

    conf.set_db_dir("/tmp/collection_manager_test");
    CollectionManager collection_manager(conf);
    collection_manager.init_collections();

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
                                          \"index\":true,\
                                          \"suggest\":true\
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
                                           \"index\":true,\
                                           \"suggest\":true\
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

     std::string schema_str_en="{\
                             \"name\":\"english_dict\",\
                             \"tokenizer\":\"default\",\
                             \"auto_suggestion\":{\
                                 \"entry_num\":5,\
                                 \"min_gram\":1,\
                                 \"max_gram\":15\
                             },\
                             \"schema\":{\
                                 \"type\":\"object\",\
                                 \"$fields\": {\
                                     \"word\":{\
                                         \"type\":\"string\",\
                                         \"index\":true\
                                       },\
                                      \"frequency\":{\
                                         \"type\":\"string\"\
                                       }\
                                   }\
                                 }\
                              }";

     //collection1
     CollectionMeta collection_meta(schema_str);
     string collection_name=collection_meta.get_collection_name();

     if(init){
         if(collection_manager.get_collection(collection_name)){
             collection_manager.drop_collection(collection_name);
         }
         cout<<collection_manager.create_collection(collection_meta)<<endl;
     }

     //collection2
     CollectionMeta collection_meta2(schema_str2);
     string collection_name2=collection_meta2.get_collection_name();
     if(init){
         if(collection_manager.get_collection(collection_name2)){
             collection_manager.drop_collection(collection_name2);
         }
         cout<<collection_manager.create_collection(collection_meta2)<<endl;
     }

     //collection3
     CollectionMeta collection_meta_en(schema_str_en);
     string collection_name_en=collection_meta_en.get_collection_name();
     if(init){
         if(collection_manager.get_collection(collection_name_en)){
             collection_manager.drop_collection(collection_name_en);
         }
         cout<<collection_manager.create_collection(collection_meta_en)<<endl;
     }

     if(init){
         {
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
         {
            std::ifstream dict_file("dataset/frequency_dictionary_en_82_765.txt");
            EXPECT_EQ(dict_file.is_open(),true);
            std::string line;
            std::string word;
            long frequency;
            size_t count=0;
            while(std::getline(dict_file,line)){
                size_t space=line.find(' ');
                word=line.substr(0,space);
                frequency=std::stol(line.substr(space+1));
                std::string doc_str="{\"id\":"+to_string(count++)+",\"word\":\""+word+"\",\"frequency\":\""+std::to_string(frequency)+"\"}";
                Document doc(doc_str);
                collection_manager.add_document(collection_name_en,doc);
            }
            std::cout<<"add english dict doc count="<<count<<std::endl;
         }
     }

    std::string search_str=R"(
                           {
                               "query": "酸菜",
                               "query by": ["recipe_name","context"],
                               "boost": [5,1],
                               "sort by": ["difficulty:asc","rank:desc"]
                           }
                           )";
    Query query("recipe",search_str);
    cout<<collection_manager.search(query)<<endl;
    std::cout<<"remove doc 4234324"<<std::endl;
    cout<<collection_manager.remove_document(collection_name,4234324)<<endl;
    std::cout<<"remove doc 543432"<<std::endl;
    cout<<collection_manager.remove_document(collection_name,543432)<<endl;
    std::cout<<"remove doc 8534571"<<std::endl;
    cout<<collection_manager.remove_document(collection_name,8534571)<<endl;

    EXPECT_FALSE(collection_manager.get_collection(collection_name)->contain_document(4234324));
    EXPECT_FALSE(collection_manager.get_collection(collection_name)->contain_document(543432));
    cout<<collection_manager.search(query)<<endl;

    std::string search_str2="{\
                           \"query\": \"鱼香柔丝盖浇饭\",\
                           \"query by\": [\"recipe_name\",\"ingredients.$items.title\"]\
                           }";
    Query query2("recipe2",search_str2);
    cout<<collection_manager.search(query2)<<endl;
    cout<<collection_manager.auto_suggest(collection_name,"经典")<<endl;
    cout<<collection_manager.auto_suggest(collection_name2,"酸菜")<<endl;

    std::string search_str_en="{\
                           \"query\": \"glass world\",\
                           \"query by\": [\"word\"]\
                           }";
    Query query_en("english_dict",search_str_en);
    cout<<"-------start fuzzy--------"<<endl;
    for(int i=0;i<1;i++){
        cout<<collection_manager.search(query_en)<<endl;
    }
    cout<<"-------end fuzzy--------"<<endl;

}
