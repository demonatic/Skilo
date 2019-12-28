#include "../src/core/art/Art.hpp"
#include <gtest/gtest.h>
#include <unordered_map>
#include <map>
#include <random>
#include <memory>
#include <chrono>
#include <stdlib.h>
#include "libart/art.h"

using namespace testing;
using namespace std;
using namespace std::chrono;

class Random {
public:
    const static  unsigned int  maxRand = std::random_device::max();
    static Random& getInstance()
    {
        static Random instance;
        return instance;
    }
    unsigned int  getInteger() noexcept {
        return (*dist)(rd);
    }
    unsigned int  GetMTEngineInteger() noexcept {
        return (*mtEngine)();
    }
    uint64_t  GetMTEngine64Integer() noexcept {
        return (*mtEngine64)();
    }

    unsigned int  GetRand0Integer() noexcept {
        return (*rand0Engine)();
    }

    auto GetRanlux48Integer() noexcept ->decltype(auto) {
        return (*ranlux48Engine)();
    }

private:
    Random() noexcept {
        mtEngine = std::make_shared<std::mt19937>(rd());
        mtEngine64 = std::make_shared<std::mt19937_64>(rd());
        dist = std::make_shared<std::uniform_int_distribution< unsigned int >>(std::uniform_int_distribution< unsigned int >(0, maxRand));
        rand0Engine = make_shared<std::minstd_rand0>(rd());
        ranlux48Engine = make_shared<std::ranlux48>(rd());
    }
    std::random_device rd;
    std::shared_ptr<std::mt19937> mtEngine;		//32-bit Mersenne Twister by Matsumoto and Nishimura, 1998
    std::shared_ptr<std::mt19937_64> mtEngine64; //64-bit Mersenne Twister by Matsumoto and Nishimura, 2000(马特赛特旋转演算法)
    std::shared_ptr<std::minstd_rand0> rand0Engine;
    std::shared_ptr<std::ranlux48> ranlux48Engine;

    std::shared_ptr<std::uniform_int_distribution< unsigned int > > dist;
};


TEST(PERFORMANCE_TEST,data) {

    std::unordered_map<string, string> dataset;
    uint32_t n = 10000000;
    cout<<"generating "<<to_string(n)<<" keys for test"<<endl;
    while (n--) {
        size_t len=8;
        string str(len,'\0');
        for(size_t i=0;i<str.size();i++){
            unsigned char val=Random::getInstance().GetMTEngine64Integer()%256;
            if(!val) val++;
            str[i]=val;
        }
        dataset[str]=str;
    }
    cout<<"generating keys done"<<endl;

    Art::ARTree<string> art;

    {
        auto start = system_clock::now();
        for(const auto &[key,val]:dataset){
            art.insert(key.c_str(),key.size(),val);
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout <<  "Art insert cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

    {
        auto start = system_clock::now();

        for(const auto &[key,val]:dataset){
            string *res=art.find(key.c_str(),key.size());
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout <<  "Art query cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

//    art_tree lib_art;
//    {
//        cout<<"start insert"<<endl;
//        auto start = system_clock::now();
//        for(const auto &[key,val]:dataset){

//        }
//        auto end   = system_clock::now();
//        auto duration = duration_cast<microseconds>(end - start);
//        cout <<  "Art insert cost "
//             << double(duration.count()) * microseconds::period::num / microseconds::period::den
//             << " seconds" << endl;
//    }

//    {
//        cout<<"start query"<<endl;
//        auto start = system_clock::now();

//        for(const auto &[key,val]:dataset){
//            string *res=art.find(key.c_str(),key.size());
//        }
//        auto end   = system_clock::now();
//        auto duration = duration_cast<microseconds>(end - start);
//        cout <<  "Lib Art query cost "
//             << double(duration.count()) * microseconds::period::num / microseconds::period::den
//             << " seconds" << endl;
//    }


    std::map<string,string> map;
    {
        auto start = system_clock::now();
        for(const auto &[key,val]:dataset){
            map.insert({key,val});
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout <<  "map insert cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

    {
        auto start = system_clock::now();
        for(const auto &[key,val]:dataset){
            auto it=map.find(key);
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);

        cout <<  "std::map query cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

    std::unordered_map<string,string> unordered_map;
    {
        auto start = system_clock::now();
        for(const auto &[key,val]:dataset){
            unordered_map.insert({key,val});
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        cout <<  "unordered_map insert cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

    {
        auto start = system_clock::now();
        for(const auto &[key,val]:dataset){
            auto it=unordered_map.find(key);
        }
        auto end   = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);

        cout <<  "std::unordered_map query cost "
             << double(duration.count()) * microseconds::period::num / microseconds::period::den
             << " seconds" << endl;
    }

}

TEST(CORRECTNESS_TST,data){
      Art::ARTree<string> art;
      std::unordered_map<string,string> data;
      while(data.size()<10000000){
          std::string str;
          size_t len=random()%23;
          for(size_t j=0;j<len;j++){
              char c=Random::getInstance().GetMTEngineInteger()%255+1;
              str.push_back(c);
          }
          data[str]=str;
      }
      cout<<"data set size="<<data.size()<<endl;
      size_t num=0;
      for(const auto &[key,val]:data){
          art.insert(key.c_str(),key.size(),val);
          string *res=art.find(key.c_str(),key.size());
          EXPECT_EQ(res!=nullptr,true);
          EXPECT_EQ(*res,val);
          if(++num%100000==0){
              cout<<"【----------"<<num<<" keys ok--------】="<<endl;
          }

      }
}

TEST(SIMPLE_TEST,data){
    Art::ARTree<const char *> art;
    const char *s1="hello";
    const char *s2="bye bye";
    const char *s3="hello too";
    const char *s4="hello shit";
    const char *s5="hello!";
    const char *s6="helloworld!";
    art.insert(s1,strlen(s1),s1);
    art.insert(s2,strlen(s2),s2);
    art.insert(s3,strlen(s3),s3);
    art.insert(s4,strlen(s4),s4);
    art.insert(s5,strlen(s5),s5);
    EXPECT_EQ(*art.find(s1,strlen(s1)),s1);
    EXPECT_EQ(*art.find(s2,strlen(s2)),s2);
    EXPECT_EQ(*art.find(s3,strlen(s3)),s3);
    EXPECT_EQ(*art.find(s4,strlen(s4)),s4);
    EXPECT_EQ(*art.find(s5,strlen(s5)),s5);
    EXPECT_EQ(art.find(s6,strlen(s6)),nullptr);
}
