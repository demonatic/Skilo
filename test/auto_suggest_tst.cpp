#include <gtest/gtest.h>
#include "core/search/AutoSuggestion.h"

using namespace testing;
using namespace std;
using namespace Skilo::Search;

TEST(AUTO_SUGGEST_TEST,ENGLISH_TEST){
    AutoSuggestor suggestor(5,2,15,20);
    suggestor.update("elevator");
    suggestor.update("speech");
    suggestor.update("elastic");
    suggestor.update("ella");
    suggestor.update("boost");
    suggestor.update("elastic");
    suggestor.update("splash");
    suggestor.update("elephant");
    suggestor.update("ella");
    suggestor.update("hello");
    suggestor.update("element");
    suggestor.update("eye");
    suggestor.update("ella");
    suggestor.update("elaeeeeeeeeeeeeeeeeeeeeeeeeee");
    suggestor.update("speech");
    suggestor.update("elephant");
    auto sug1=suggestor.auto_suggest("el");
    EXPECT_EQ(sug1[0],"ella");
    EXPECT_EQ(sug1[1],"elastic");
    EXPECT_EQ(sug1[2],"elephant");
    EXPECT_EQ(sug1[3],"element");
    EXPECT_EQ(sug1[4],"elevator");

    auto sug2=suggestor.auto_suggest("ele");
    EXPECT_EQ(sug2[0],"elephant");
    EXPECT_EQ(sug2[1],"element");
    EXPECT_EQ(sug2[2],"elevator");

    auto sug3=suggestor.auto_suggest("sp");
    EXPECT_EQ(sug3[0],"speech");
    EXPECT_EQ(sug3[1],"splash");

    auto sug4=suggestor.auto_suggest("elae");
    EXPECT_EQ(sug4.empty(),true);

    auto sug5=suggestor.auto_suggest("e");
    EXPECT_EQ(sug5.empty(),true);
}

TEST(AUTO_SUGGEST_TEST,CHINESE_TEST){
    AutoSuggestor suggestor(5);
    suggestor.update("我爱你C++");
    suggestor.update("中国足球");
    suggestor.update("中国女排");
    suggestor.update("红烧排骨真好吃");
    suggestor.update("中国女排");
    suggestor.update("我爱你C++");
    suggestor.update("我爱你汇编");
    suggestor.update("红烧翅根做起来很简单");
    suggestor.update("中国女排");
    suggestor.update("中国足球");
    suggestor.update("我爱你C++");
    suggestor.update("我爱你Java");
    suggestor.update("红烧排骨真好吃");
    suggestor.update("我爱你Java");
    suggestor.update("我爱你的笑容");
    suggestor.update("中国台湾");

    auto sug1=suggestor.auto_suggest("中国");
    EXPECT_EQ(sug1[0],"中国女排");
    EXPECT_EQ(sug1[1],"中国足球");
    EXPECT_EQ(sug1[2],"中国台湾");

    auto sug2=suggestor.auto_suggest("红烧");
    EXPECT_EQ(sug2[0],"红烧排骨真好吃");
    EXPECT_EQ(sug2[1],"红烧翅根做起来很简单");


    auto sug3=suggestor.auto_suggest("我爱你");
    EXPECT_EQ(sug3[0],"我爱你C++");
    EXPECT_EQ(sug3[1],"我爱你Java");
    EXPECT_EQ(sug3[2],"我爱你的笑容");
    EXPECT_EQ(sug3[3],"我爱你汇编");

    auto sug4=suggestor.auto_suggest("红烧肉");
    EXPECT_EQ(sug4.empty(),true);

    auto sug5=suggestor.auto_suggest("我爱coding");
    EXPECT_EQ(sug5.empty(),true);

    suggestor.update("酸菜鱼");
    EXPECT_TRUE(suggestor.auto_suggest("酸菜")[0]=="酸菜鱼");
    cout<<suggestor.auto_suggest("酸菜")[0]<<endl;
}






