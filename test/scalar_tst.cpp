#include <gtest/gtest.h>
#include <unordered_set>
#include "../../src/core/index/CompressedScalar.hpp"

using namespace testing;
using namespace std;
using namespace Skilo::Index;

TEST(SCALAR_TEST,SORTED_CRUD_TEST) {
    CompressedScalar<ScalarType::Sorted> scalar;
    vector<uint32_t> data;
    uint32_t n=30;
    for(uint32_t i=0;i<n;i++){
        data.push_back(i);
        scalar.append(i);
    }
    std::unique_ptr<uint32_t[]> uncompressed_data=scalar.uncompress();
    for(uint32_t i=0;i<n;i++){
        EXPECT_TRUE(scalar.contains(i));
        EXPECT_EQ(scalar.index_of(i),i);
        EXPECT_TRUE(scalar[i]==data[i]&&data[i]==uncompressed_data[i]&&data[i]==scalar.at(i));
        EXPECT_EQ(scalar.at(i),i);
        cout<<"i="<<i<<" scalar="<<scalar[i]<<" uncompress="<<uncompressed_data[i]<<" data="<<data[i]<<endl;
    }
    EXPECT_EQ(scalar.index_of(n+1),scalar.length());
}

TEST(SCALAR_TEST,UNSORTED_CRUD_TEST) {
    CompressedScalar<ScalarType::UnSorted> scalar;
    vector<uint32_t> data;
    unordered_set<uint32_t> randoms;
    uint32_t n=30;
    for(uint32_t i=0;i<n;i++){
GENERATE:
         uint32_t num=rand()%1000;
         auto it=randoms.find(num);
         if(it!=randoms.end()){
             goto GENERATE;
         }
         randoms.emplace(num);
    }

    for(uint32_t num:randoms){
        data.push_back(num);
        scalar.append(num);
    }
    std::unique_ptr<uint32_t[]> uncompressed_data=scalar.uncompress();
    for(uint32_t i=0;i<n;i++){
        EXPECT_TRUE(scalar.contains(data[i]));
        EXPECT_EQ(scalar.index_of(data[i]),i);
        EXPECT_TRUE(scalar[i]==data[i]&&data[i]==uncompressed_data[i]&&data[i]==scalar.at(i));
        EXPECT_EQ(scalar.at(i),data[i]);
        cout<<"i="<<i<<" scalar="<<scalar[i]<<" uncompress="<<uncompressed_data[i]<<" data="<<data[i]<<endl;
    }
    EXPECT_EQ(scalar.index_of(1001),scalar.length());
}
