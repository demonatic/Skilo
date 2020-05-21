#ifndef FUNCTIONTIMER_H
#define FUNCTIONTIMER_H

#include <chrono>
#include <cstdio>
#include <string>

namespace Skilo {


#define timing_code(__code_block__) \
    do { \
        auto start = std::chrono::system_clock::now();\
        __code_block__;\
        auto end   = std::chrono::system_clock::now();\
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);\
       printf("function costs %f s\n", double(duration.count()) *  std::chrono::microseconds::period::num /  std::chrono::microseconds::period::den);\
    } while (0)
}

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
}


#endif // FUNCTIONTIMER_H
