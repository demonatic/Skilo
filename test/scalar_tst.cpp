#include <gtest/gtest.h>
#include <unordered_set>
#include "../../src/core/index/CompressedScalar.hpp"

using namespace testing;
using namespace std;
using namespace Skilo::Index;

template<ScalarType T>
void expect_equal(vector<uint32_t> &data,CompressedScalar<T> &scalar){
    std::unique_ptr<uint32_t[]> uncompressed_data=scalar.uncompress();
    for(uint32_t i=0;i<data.size();i++){
        EXPECT_TRUE(scalar.contains(data[i]));
        EXPECT_EQ(scalar.index_of(data[i]),i);
        EXPECT_TRUE(scalar[i]==data[i]&&data[i]==uncompressed_data[i]&&data[i]==scalar.at(i));
        EXPECT_EQ(scalar.at(i),data[i]);
        //cout<<"i="<<i<<" scalar="<<scalar[i]<<" uncompress="<<uncompressed_data[i]<<" data="<<data[i]<<endl;
    }
    EXPECT_EQ(data.size(),scalar.length());
}


TEST(SCALAR_TEST,SORTED_CRUD_TEST) {
    CompressedScalar<ScalarType::Sorted> scalar;
    vector<uint32_t> data;
    uint32_t n=1000;
    for(uint32_t i=0;i<n;i++){
        data.push_back(i);
        scalar.append(i);
    }
    expect_equal(data,scalar);
    EXPECT_EQ(scalar.index_of(n+1),scalar.length());

    size_t rm_begin=700,rm_end=999;
    scalar.remove_range(rm_begin,rm_end);
    data.erase(data.begin()+rm_begin,data.begin()+rm_end);
    scalar.remove_value(data.back());
    data.pop_back();
    expect_equal(data,scalar);

    for(uint32_t i=0;i<n;i++){
        uint32_t val=data.size();
        data.push_back(val);
        scalar.append(val);
    }
    expect_equal(data,scalar);
    EXPECT_EQ(scalar.index_of(data.size()),scalar.length());

    auto f=[](uint32_t v){return v*=2;};
    size_t trans_start=n/3,trans_end=scalar.length();
    for(size_t i=trans_start;i<trans_end;i++){
        data[i]=f(data[i]);
    }
    scalar.apply(trans_start,trans_end,f);
    std::unique_ptr<uint32_t[]> uncompressed_data=scalar.uncompress();
    for(uint32_t i=0;i<trans_start;i++){
        EXPECT_TRUE(scalar.contains(data[i]));
        EXPECT_EQ(scalar.index_of(data[i]),i);
        EXPECT_TRUE(scalar[i]==data[i]&&data[i]==uncompressed_data[i]&&data[i]==scalar.at(i));
        EXPECT_EQ(scalar.at(i),data[i]);
    }
    for(uint32_t i=trans_start;i<trans_end;i++){
        EXPECT_TRUE(scalar.contains(data[i]));
        EXPECT_TRUE(scalar[i]==data[i]&&data[i]==uncompressed_data[i]&&data[i]==scalar.at(i));
        EXPECT_EQ(scalar.at(i),data[i]);
    }
    EXPECT_EQ(data.size(),scalar.length());
}

TEST(SCALAR_TEST,UNSORTED_CRUD_TEST) {
    CompressedScalar<ScalarType::UnSorted> scalar;
    vector<uint32_t> data;
    unordered_set<uint32_t> randoms;
    uint32_t n=1000;
    for(uint32_t i=0;i<n;i++){
GENERATE:
         uint32_t num=rand();
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
    expect_equal(data,scalar);
    EXPECT_EQ(scalar.index_of(1001),scalar.length());

    size_t rm_begin=700,rm_end=999;
    scalar.remove_range(rm_begin,rm_end);
    data.erase(data.begin()+rm_begin,data.begin()+rm_end);
    scalar.remove_value(data.back());
    data.pop_back();
    expect_equal(data,scalar);

    auto f=[](uint32_t v){return v+2;};
    size_t trans_start=n/3,trans_end=n/2;
    for(size_t i=trans_start;i<trans_end;i++){
        data[i]=f(data[i]);
    }
    scalar.apply(trans_start,trans_end,f);
    expect_equal(data,scalar);
}
