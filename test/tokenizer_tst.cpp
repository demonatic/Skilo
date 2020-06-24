#include <gtest/gtest.h>
#include "core/index/Tokenizer.h"

using namespace testing;
using namespace std;
using namespace Skilo;
using namespace Skilo::Index;

TEST(TOKENIZER_TEST,JIEBA_TEST) {

    JiebaTokenizer tokenizer("../src/dict");
    string s1="我是手扶拖拉机专业的,21世纪是拖拉机的世纪";
    std::unordered_map<std::string, std::vector<uint32_t>> r1=tokenizer.tokenize(s1).term_to_offsets();
    cout<<"<---sentence="<<s1<<"--->"<<endl;
    for(const auto &[word,offsets]:r1){

        cout<<"word="<<word<<" offsets=";
        EXPECT_TRUE(word!="的");
        EXPECT_TRUE(word!="是");
        for(auto &off:offsets){
            cout<<off<<" ";
        }
    }
    cout<<endl;
    EXPECT_TRUE(r1.count("专业")!=0);
    EXPECT_TRUE(r1.count("拖拉机")!=0);

    string s2="牛肉含有丰富的蛋白质，氨基酸组成等比猪肉更接近人体需要，能提高机体抗病能力";
    std::unordered_map<std::string, std::vector<uint32_t>> r2=tokenizer.tokenize(s2).term_to_offsets();
    cout<<"<---sentence="<<s2<<"--->"<<endl;
    for(const auto &[word,offsets]:r2){

        cout<<"word="<<word<<" offsets=";
        EXPECT_TRUE(word!="的");
        EXPECT_TRUE(word!="能");
        for(auto &off:offsets){
            cout<<off<<" ";
        }
    }
    cout<<endl;
    EXPECT_TRUE(r2.count("牛肉")!=0);
    EXPECT_TRUE(r2.count("蛋白质")!=0);
    EXPECT_TRUE(r2.count("氨基酸")!=0);
    EXPECT_TRUE(r2.count("丰富")!=0);
    EXPECT_TRUE(r2.count("猪肉")!=0);
    EXPECT_TRUE(r2.count("含有")!=0);
    EXPECT_TRUE(r2.count("能力")!=0);
}
