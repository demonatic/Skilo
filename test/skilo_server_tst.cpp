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

TEST(SKILO_SERVER_TEST,CRUD_TEST){
    SkiloConfig config;
    SkiloServer server(config);
    EXPECT_TRUE(server.listen());
}

#define PORT 8080
#define SERV "127.0.0.1"
#define BUFF 65535

void send_request_to_server(){
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

    bool init_collection=true;
    bool search=true;
    if(init_collection){
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
        string body_len_str=to_string(req_body.size());
        std::string req= "POST /collections HTTP/1.1\r\n"
                         "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;

        memcpy(sendbuff,req.data(),req.length());
        send(sockfd,sendbuff,req.length(),0);
        recv(sockfd, recvbuff, sizeof(recvbuff), 0);
        fputs(recvbuff,stdout);
    }

    if(init_collection){
            std::ifstream file("dataset/document.json",ios::in|ios::ate);
            size_t size=file.tellg();
            file.seekg(0,ios::beg);
            string req_body(size,'\0');
            file.read(req_body.data(),size);
            file.close();

            string body_len_str=to_string(req_body.size());
            std::string req= "POST /collections/recipe/documents HTTP/1.1\r\n"
                             "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;

            memcpy(sendbuff,req.data(),req.length());
            send(sockfd,sendbuff,req.length(),0);
            recv(sockfd, recvbuff, sizeof(recvbuff), 0);
            fputs(recvbuff,stdout);
    }

    if(search){
        string req_body= "{\
                         \"query\": \"酸菜鱼\",\
                         \"query by\": [\"recipe_name\",\"context\"]\
                         }";

        string body_len_str=to_string(req_body.size());
        std::string req= "GET /collections/recipe/documents HTTP/1.1\r\n"
                         "Content-Length: "+body_len_str+"\r\n\r\n"+req_body;

        memcpy(sendbuff,req.data(),req.length());
        send(sockfd,sendbuff,req.length(),0);
        recv(sockfd, recvbuff, sizeof(recvbuff), 0);
        fputs(recvbuff,stdout);

    }
    close(sockfd);
}


