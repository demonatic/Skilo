#ifndef COMPRESSABLEARRAY_H
#define COMPRESSABLEARRAY_H

#include <cstring>
#include <memory>

/// @class an integer array use 'Frame Of Reference' compression
template<typename Integer=uint32_t,bool sorted=false,typename=std::enable_if_t<std::is_integral_v<Integer>>>
class CompressableArray
{
public:
    static constexpr size_t DataSize=sizeof(Integer);

public:
    CompressableArray(const uint32_t n=4){
        _capacity=n*DataSize;
        _data=static_cast<Integer*>(malloc(_capacity));
        _length=0;
    }
    
    ~CompressableArray(){
        free(_data);
        _data=nullptr;
    }

    size_t size(){
        return _length;
    }

    size_t capacity(){
        return _capacity;
    }

    void append(const Integer val){
        if constexpr(sorted){

        }
        else{

        }
        _length++;
    }

    std::unique_ptr<Integer*> uncompress(){

    }

protected:
    Integer *_data;
    size_t _length;
    size_t _capacity;
};

#endif // COMPRESSABLEARRAY_H
