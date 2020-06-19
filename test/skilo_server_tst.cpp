#include <gtest/gtest.h>
#include "Skilo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>

using namespace testing;
using namespace std;
using namespace Skilo;

#define PORT 8983
#define SERV "127.0.0.1"
#define BUFF 655350

void send_request_to_server(bool init_collection,bool search);


void init_collection_client(){
    sleep(2);
    send_request_to_server(true,false);
}
void search_client(){
    sleep(5);
    send_request_to_server(false,true);
}

TEST(SKILO_SERVER_TEST,CRUD_TEST){
    SkiloConfig config;
    config.set_listen_port(PORT);
    config.set_db_dir("/tmp/skilo_server_tst");
    SkiloServer server(config,true);
    size_t client_count=8;
    std::vector<std::thread> client_threads;
    for(int i=0;i<client_count;i++){
        if(i==0){
            client_threads.push_back(std::thread(&init_collection_client));
        }
        else {
            client_threads.push_back(std::thread(&search_client));
        }
    }
    client_threads.push_back(std::thread([&](){
        sleep(8);
        server.stop();
    }));
    EXPECT_TRUE(server.listen());
    for(auto &t:client_threads){
        t.join();
    }
}

#define TST_REQ(req_str){\
    memcpy(sendbuff,req_str.data(),req_str.length());\
    send(sockfd,sendbuff,req_str.length(),0);\
    recv(sockfd, recvbuff, sizeof(recvbuff), 0);\
    fputs(recvbuff,stdout);\
    memset(recvbuff,'\0',sizeof(recvbuff));\
}

void send_request_to_server(bool init_collection,bool search){
    // 定义socket
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    // 定义sockaddr_in
    struct sockaddr_in skaddr;
    skaddr.sin_family = AF_INET;
    skaddr.sin_port   = htons(PORT);
    skaddr.sin_addr.s_addr = inet_addr(SERV);

    if( connect(sockfd,(struct sockaddr *)&skaddr, sizeof(skaddr)) < 0 ){
        perror("connect error");
        exit(1);
    }

    char sendbuff[BUFF];
    char recvbuff[BUFF];


    if(init_collection){
        string req_drop="DELETE /collections/recipe HTTP/1.1\r\n"
                       "Host: Chrome\r\n\r\n";

        memcpy(sendbuff,req_drop.data(),req_drop.length());
        send(sockfd,sendbuff,req_drop.length(),0);
        recv(sockfd, recvbuff, sizeof(recvbuff), 0);
        fputs(recvbuff,stdout);

        string req_body= "{\
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
                                       \"index\":true,\
                                       \"suggest\":true\
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
                             },\
                            \"auto_suggestion\":{\
                                 \"entry_num\":5,\
                                 \"min_gram\":1,\
                                 \"max_gram\":15\
                             }\
                          }";
        string body_len_str=to_string(req_body.size());
        std::string req_create= "POST /collections HTTP/1.1\r\n"
                         "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;

        TST_REQ(req_create);
    }

    if(init_collection){
            std::ifstream file("dataset/document.json",ios::in|ios::ate);
            size_t size=file.tellg();
            file.seekg(0,ios::beg);
            string req_body(size,'\0');
            file.read(req_body.data(),size);
            file.close();
            string body_len_str=to_string(req_body.size());
            std::string req= "POST /collections/recipe HTTP/1.1\r\n"
                             "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;

            TST_REQ(req);

            string req_delete="DELETE /collections/recipe/543432 HTTP/1.1\r\n"
                           "Host: Chrome\r\n\r\n";
            TST_REQ(req_delete);
    }

    if(search){
        string req_body= "{\
                         \"query\": \"酸菜\",\
                         \"query by\": [\"recipe_name\",\"context\"]\
                         }";

        string body_len_str=to_string(req_body.size());
        std::string query_req= "GET /collections/recipe/documents HTTP/1.1\r\n"
                         "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;
        TST_REQ(query_req);

        string sug_req="GET /collections/recipe/auto_suggestion?q=%e9%85%b8 HTTP/1.1\r\n"
                       "Host: Chrome\r\n\r\n";
        TST_REQ(sug_req);

        string overall_summary_req="GET /collections HTTP/1.1\r\n"
                       "Content-Length: 0\r\n\r\n";
        TST_REQ(overall_summary_req);

        string collec_summary_req="GET /collections/recipe HTTP/1.1\r\n"
                       "Content-Length: 0\r\n\r\n";
        TST_REQ(collec_summary_req);
    }
    close(sockfd);
}


