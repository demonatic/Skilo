#include "../src/core/index/Art.hpp"
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


TEST(ART_TEST,PERFORMANCE_TEST) {

    std::unordered_map<string, string> dataset;
    uint32_t n = 1000; //10000000
    size_t len=16;
    cout<<"generating "<<to_string(n)<<" keys if length "<<len<<" for test"<<endl;
    while (n--) {
        string str(len,'\0');
        for(size_t i=0;i<str.size();i++){
            unsigned char val=Random::getInstance().GetMTEngineInteger()%256;
            if(!val) val++;
            str[i]=val;
        }
        dataset[str]=str+str;
    }
    cout<<"generating keys done"<<endl;

    {
        Art::ARTree<string> art;

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                art.insert(key.c_str(),key.size(),val);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "My Art insert cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                void *res=art.find(key.c_str(),key.size());
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "My Art query cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                art.erase(key.c_str(),key.size());
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "My Art deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
    }

    {
        art_tree lib_art;
        int init_res=art_tree_init(&lib_art);
        EXPECT_EQ(init_res,0);
        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                art_insert(&lib_art,(unsigned char*)key.c_str(),key.size()+1,new std::string(val));
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "Lib Art insert cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                void *res=art_search(&lib_art,(unsigned char*)(key.c_str()),key.size()+1);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "Lib Art query cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                void *res=art_delete(&lib_art,(unsigned char*)(key.c_str()),key.size()+1);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "Lib Art deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
        art_tree_destroy(&lib_art);
    }

    {
        std::map<string,string> map;
        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                map.insert({key,val});
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "std::map insert cost "
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

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                auto it=map.erase(key);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);

            cout <<  "std::map deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
    }

    {
        std::unordered_map<string,string> unordered_map;
        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                unordered_map.insert({key,val});
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "std::unordered_map insert cost "
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

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                auto it=unordered_map.erase(key);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);

            cout <<  "std::unordered_map deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
    }
}

TEST(ART_TEST,CORRECTNESS_TST){
      Art::ARTree<string> art;
      std::unordered_map<string,string> data;
      while(data.size()<1000){
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
              cout<<"【----------check insertion of "<<num<<" keys ok--------】"<<endl;
          }
      }
      num=0;
      for(const auto &[key,val]:data){
           art.erase(key.c_str(),key.size());
           string *res=art.find(key.c_str(),key.size());
           EXPECT_EQ(res==nullptr,true);
           if(++num%100000==0){
               cout<<"【----------check deletion of "<<num<<" keys ok--------】"<<endl;
           }
      }
}

TEST(ART_TEST,SIMPLE_TEST){
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
    art.erase(s3,strlen(s3));
    EXPECT_EQ(art.find(s3,strlen(s3)),nullptr);
}
