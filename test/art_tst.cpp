#include "../src/core/index/Art.hpp"
#include <gtest/gtest.h>
#include <unordered_map>
#include <map>
#include <random>
#include <memory>
#include <chrono>
#include <stdlib.h>

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
    uint32_t n = 100000; //10000000
    std::vector<std::pair<string,string>> dataset(n);
    {
        std::unordered_map<string, string> set(n);

        size_t len=15;
        cout<<"generating "<<to_string(n)<<" keys  length="<<len<<" for test"<<endl;
        while (n--) {
            string str(len,'\0');
            for(size_t i=0;i<str.size();i++){
                unsigned char val=Random::getInstance().GetMTEngineInteger()%256;
                if(!val) val++;
                str[i]=val;
            }
            set[str]=str+str;
        }
        for(const auto &[k,v]:set){
            dataset.emplace_back(k,v);
        }
    }

    cout<<"generating keys done"<<endl;

    {
        Art::ARTree<string> art;

        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                art.insert(key.data(),key.size(),new std::string(val));
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
                string *res=art.find(key.data(),key.size());
                assert(*res==val);
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
                art.erase(key.data(),key.size());
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);
            cout <<  "My Art deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
    }

    {
        std::unordered_map<string,string> unordered_map;
        {
            auto start = system_clock::now();
            for(const auto &[key,val]:dataset){
                unordered_map.emplace(key,val);
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
                assert(it->second==val);
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
                unordered_map.erase(key);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);

            cout <<  "std::unordered_map deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
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
                map.erase(key);
            }
            auto end   = system_clock::now();
            auto duration = duration_cast<microseconds>(end - start);

            cout <<  "std::map deletion cost "
                 << double(duration.count()) * microseconds::period::num / microseconds::period::den
                 << " seconds" << endl;
        }
    }
}

TEST(ART_TEST,CORRECTNESS_TST){
      Art::ARTree<string> art;
      std::unordered_map<string,string> data;
      while(data.size()<100000){
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
          art.insert(key.c_str(),key.size(),new std::string(val));
          string *res=art.find(key.c_str(),key.size());
          EXPECT_EQ(res!=nullptr,true);
          EXPECT_EQ(*res,val);
          if(++num%100000==0){
              cout<<"【----------check insertion of "<<num<<" keys ok--------】"<<endl;
          }
      }
      size_t it_count=0;
      art.iterate("",0,[&it_count](unsigned char *key,size_t len,std::string* s){
          it_count++;
      });
      EXPECT_EQ(it_count,100000);
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
    Art::ARTree<std::string> art;
     string *s1=new string("hello");
     string *s2=new string("bye bye");
     string *s3=new string("hello boo");
     string *s4=new string("hello shit");
     string *s5=new string("hello!");
     string *s6=new string("helloworld!");
     art.insert(s1->data(),s1->length(),s1);
     art.insert(s2->data(),s2->length(),s2);
     art.insert(s3->data(),s3->length(),s3);
     art.insert(s4->data(),s4->length(),s4);
     art.insert(s5->data(),s5->length(),s5);
//     art.insert(s6->data(),s6->length(),s6);
     EXPECT_EQ(art.find(s1->data(),s1->length()),s1);
     EXPECT_EQ(art.find(s2->data(),s2->length()),s2);
     EXPECT_EQ(art.find(s3->data(),s3->length()),s3);
     EXPECT_EQ(art.find(s4->data(),s4->length()),s4);
     EXPECT_EQ(art.find(s5->data(),s5->length()),s5);
     EXPECT_EQ(art.find(s6->data(),s6->length()),nullptr);

     std::vector<std::string*> iterate_res1,iterate_res2;
     const char* prefix="hello";
     art.iterate(prefix,strlen(prefix),[&](unsigned char *key,size_t len,std::string *s){
         std::cout<<*s<<std::endl;
         iterate_res1.push_back(s);
     },[](auto c){
         return c=='b'?true:false;
     });
     EXPECT_EQ(iterate_res1.size(),3);
     EXPECT_EQ(iterate_res1[0],s1);
     EXPECT_EQ(iterate_res1[1],s4);
     EXPECT_EQ(iterate_res1[2],s5);

     const char* prefix2="helloo";
     art.iterate(prefix2,strlen(prefix2),[&](unsigned char *key,size_t len,std::string *s){
         iterate_res2.push_back(s);
     });
     EXPECT_EQ(iterate_res2.size(),0);

     std::string t_s3=*s3;
     art.erase(s3->data(),s3->length());
     EXPECT_EQ(art.find(t_s3.data(),t_s3.length()),nullptr);
}
