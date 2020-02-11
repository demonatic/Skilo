#include <gtest/gtest.h>
#include "../../src/core/Document.h"
#include <cstring>

using namespace testing;
using namespace std;
using namespace Skilo;

TEST(DOCUMENT_TEST,PARSE_TEST){
    SegmentBuf seg_buf;
    string seg1="{\"sites\":[";
    string seg2="{\"name\":\"Runoob\", \"url\":\"www.runoob.com\"},";
    string seg3="{\"name\":\"Google\", \"url\":\"www.google.com\"},";
    string seg4="{\"name\":\"Taobao\", \"url\":\"www.taobao.com\"}";
    string seg5="]}";
    string json_str=seg1+seg2+seg3+seg4+seg5;

    seg_buf.push_back({(uint8_t*)seg1.c_str(),seg1.size()});
    seg_buf.push_back({(uint8_t*)seg2.c_str(),seg2.size()});
    seg_buf.push_back({(uint8_t*)seg3.c_str(),seg3.size()});
    seg_buf.push_back({(uint8_t*)seg4.c_str(),seg4.size()});
    seg_buf.push_back({(uint8_t*)seg5.c_str(),seg5.size()});
    Document document(json_str);
    Document document2(seg_buf);
    EXPECT_EQ(document.get_raw(),document2.get_raw());
    EXPECT_EQ(document2.get_raw()["sites"].Size(),3);
    EXPECT_EQ(string(document2.get_raw()["sites"][0]["name"].GetString()),"Runoob");
    EXPECT_EQ(string(document2.get_raw()["sites"][1]["name"].GetString()),"Google");
    EXPECT_EQ(string(document2.get_raw()["sites"][2]["name"].GetString()),"Taobao");
    EXPECT_EQ(string(document2.get_raw()["sites"][0]["url"].GetString()),"www.runoob.com");
    EXPECT_EQ(string(document2.get_raw()["sites"][1]["url"].GetString()),"www.google.com");
    EXPECT_EQ(string(document2.get_raw()["sites"][2]["url"].GetString()),"www.taobao.com");
}
