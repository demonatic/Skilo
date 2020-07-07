#ifndef FUNCTIONTIMER_H
#define FUNCTIONTIMER_H

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>
#include <numeric>
#include <functional>
#include <cassert>
#include "utility/Exception.h"

namespace Skilo {

using std::string;
using std::pair;

namespace Util{
    /// @brief remove all 'c' in str
    inline void trim(std::string &str,const char c){
        int j=0;
        for(int i=0;i<str.size();i++){
            if(str[i]!=c){
                str[j++]=str[i];
            }
        }
        str.erase(j);
    }

    template<typename T,typename F>
    inline void cartesian(std::vector<std::vector<T>> &vec,F on_choice,size_t max_combination=std::numeric_limits<size_t>::max(),std::function<bool()> early_termination=[](){return false;}){
        const size_t N = std::accumulate(vec.begin(),vec.end(),1LL,[](size_t a, std::vector<T>& b){
            return a*b.size();
        });

        std::vector<T> u(vec.size());
        for(auto n=0; n<std::min(max_combination,N); n++){
          lldiv_t q{n,0};
          for(int i=vec.size()-1;i>=0;i--) {
            q=div(q.quot,vec[i].size());
            u[i]=vec[i][q.rem];
            if(early_termination()){
                return;
            }
          }
          on_choice(u);
        }
    }

    inline size_t get_utf8_char_len(const char c)
    {
        size_t len=0;
        if((c&0x80)==0){ //0xxx xxxx -> ascii
            len=1;
        }
        else if((c&0xf8)==0xf8){ // 111110xx
            len=5;
        }
        else if((c&0xf0)==0xf0){ // 11110xxx
            len=4;
        }
        else if((c&0xe0)==0xe0){ // 1110xxxx
            len=3;
        }
        else if((c&0xc0)==0xc0){ // 11xxxxxx
            len=2;
        }
        else{
            throw InvalidFormatException("string should be uft-8 or ascii");
        }
        return len;
    }

    inline size_t utf8_len(const std::string &str){
        size_t utf8_len=0;
        for(size_t i=0;i<str.size();i+=get_utf8_char_len(str[i])){
            utf8_len++;
        }
        return utf8_len;
    }

    inline bool is_chinese(const std::string &term){
        //TODO
        return term[0]<0;
    }

    template<class F,class ...Args>
    inline float timing_function(F &&f, Args&& ...args){
        auto start = std::chrono::system_clock::now();
        f(std::forward<Args>(args)...);
        auto end=std::chrono::system_clock::now();
        auto duration=std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return double(duration.count())*std::chrono::microseconds::period::num/std::chrono::microseconds::period::den;
    }

#define timing_code_block(__code_block__) \
    do { \
        auto start = std::chrono::system_clock::now();\
        __code_block__;\
        auto end = std::chrono::system_clock::now();\
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);\
        printf("function costs %f s\n", double(duration.count())*std::chrono::microseconds::period::num/std::chrono::microseconds::period::den);\
    } while (0)
}

}


#endif // FUNCTIONTIMER_H
