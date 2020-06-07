#ifndef COMPRESSABLEARRAY_H
#define COMPRESSABLEARRAY_H

#include <cstdlib>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cassert>
#include <functional>
#include <for.h>
#include <iostream>
namespace Skilo{
namespace Index{

enum class ScalarType{
    Sorted,
    UnSorted,
};

/// @class an uint32 array use "Frame Of Reference" compression
template<ScalarType type>
class CompressedScalar
{
public:
    static constexpr size_t ElementSize=sizeof(uint32_t);
    static constexpr size_t MEDATA_OVERHEAD=5;
    static constexpr double GrowthFactor=1.3;

public:
    CompressedScalar(const uint32_t n=2){
        _byte_capacity=MEDATA_OVERHEAD+n*ElementSize;
        _data=static_cast<uint8_t*>(std::calloc(_byte_capacity,1));
    }
    
    ~CompressedScalar(){
        free(_data);
        _data=nullptr;
    }

    void append(const uint32_t value){
        uint32_t size_required=this->get_append_size_required(value, _elm_count+1);
        if(size_required+ElementSize>_byte_capacity){
            size_t new_size=(size_t) size_required*GrowthFactor;

            uint8_t *new_addr=static_cast<uint8_t*>(std::realloc(_data,new_size));
            if(!new_addr){
                throw std::bad_alloc();
            }
            _data=new_addr;
            _byte_capacity=new_size;
        }

        uint32_t new_length;
        if constexpr(type==ScalarType::Sorted){
            new_length=for_append_sorted(_data,_elm_count,value);
        }
        else{
            new_length=for_append_unsorted(_data,_elm_count,value);
        }
        if(!new_length){ //re-encond trigerred and failed due to malloc failure
            throw std::bad_alloc();
        }

        _min_val=std::min(_min_val,value);
        _max_val=std::max(_max_val,value);
        _byte_length = new_length;
        _elm_count++;
    }

    /// Uncompresses the entire sequence of ints in the array and return it
    std::unique_ptr<uint32_t[]> uncompress() const{
        std::unique_ptr<uint32_t[]> uncompressed=std::make_unique<uint32_t[]>(_elm_count);
        for_uncompress(_data,uncompressed.get(),_elm_count);
        return uncompressed;
    }

    uint32_t at(const uint32_t index) const{
        if(index<0||index>=_elm_count){
            throw std::out_of_range("index is out of scalar range");
        }
        return (*this)[index];
    }

    uint32_t operator[](const uint32_t index) const{
        return for_select(_data,index);
    }

    bool contains(const uint32_t value) const{
        if constexpr(type==ScalarType::Sorted){
            uint32_t actual;
            for_lower_bound_search(_data,_elm_count,value,&actual);
            return actual==value;
        }
        else{
            uint32_t index=for_linear_search(_data,_elm_count,value);
            return index!=_elm_count;
        }
    }

    /// @return the index of the found element, or 'length' if the key was not found.
    uint32_t index_of(const uint32_t value) const{
        if constexpr(type==ScalarType::Sorted){
            if(!_elm_count) return _elm_count;
            uint32_t actual;
            uint32_t index=for_lower_bound_search(_data,_elm_count,value,&actual);
            return actual==value?index:_elm_count;
        }
        else{
            return for_linear_search(_data,_elm_count,value);
        }
    }

    /// @brief remove all elements between [begin,end)
    void remove_range(const size_t begin,const size_t end){
        assert(begin<=end&&end<=this->length());
        if(begin==end)
            return;

        size_t new_length=this->length()-(end-begin);

        _max_val=std::numeric_limits<uint32_t>::min();
        _min_val=std::numeric_limits<uint32_t>::max();

        std::unique_ptr<uint32_t[]> new_elms=std::make_unique<uint32_t[]>(new_length);
        std::unique_ptr<uint32_t[]> origin_elms=this->uncompress();

        for(size_t i=0,j=0;i<this->length();i++){
            if(i<begin||i>=end){
                uint32_t val=origin_elms[i];
                _min_val=std::min(_min_val,val);
                _max_val=std::max(_max_val,val);
                new_elms[j++]=val;
            }
        }

        _byte_capacity=this->get_append_size_required(_max_val,new_length)*GrowthFactor;
        uint8_t *new_addr=static_cast<uint8_t*>(std::realloc(_data,_byte_capacity));
        if(!new_addr){
           throw std::bad_alloc();
        }
        _data=new_addr;

        if constexpr(type==ScalarType::Sorted){
            _byte_length=for_compress_sorted(new_elms.get(),_data,new_length);
        }
        else{
            _byte_length=for_compress_unsorted(new_elms.get(),_data,new_length);
        }
        _elm_count=new_length;
    }

    void remove(const size_t index){
        this->remove_range(index,index+1);
    }

    void remove_value(const uint32_t val){
        uint32_t index=this->index_of(val);
        if(index==this->length()){
            return;
        }
        this->remove(index);
    }

    /// @brief apply each element between [begin,end) with given transformer
    void apply(const size_t begin,const size_t end,std::function<uint32_t(uint32_t)> transformer){
        assert(begin<=end&&end<=this->length());
        if(begin==end)
            return;

        _max_val=std::numeric_limits<uint32_t>::min();
        _min_val=std::numeric_limits<uint32_t>::max();

        std::unique_ptr<uint32_t[]> elms=this->uncompress();
        for(size_t i=0;i<begin;i++){
            _min_val=std::min(_min_val,elms[i]);
            _max_val=std::max(_max_val,elms[i]);
        }
        for(size_t i=begin;i<end;i++){
            uint32_t new_val=transformer(elms[i]);
            elms[i]=new_val;
            _min_val=std::min(_min_val,new_val);
            _max_val=std::max(_max_val,new_val);
        }

        _byte_capacity=this->get_append_size_required(_max_val,_elm_count)*GrowthFactor;
        uint8_t *new_addr=static_cast<uint8_t*>(std::realloc(_data,_byte_capacity));
        if(!new_addr){
           throw std::bad_alloc();
        }
        _data=new_addr;
        if constexpr(type==ScalarType::Sorted){
             _byte_length=for_compress_sorted(elms.get(),_data,_elm_count);
#ifndef NDEBUG
    for(int i=1;i<_elm_count;i++){
        if(elms[i]<elms[i-1]){
            throw std::runtime_error("the scalar is not sorted after transformed");
        }
    }
#endif
        }
        else{
            _byte_length=for_compress_unsorted(elms.get(),_data,_elm_count);
        }
    }

    /// @return num of elements
    uint32_t length() const{
        return _elm_count;
    }

    /// @return num of bytes has allocated
    size_t bytes_capacity() const{
        return _byte_capacity;
    }

    size_t bytes_length() const{
        return _byte_length;
    }

private:
    /// @brief get the least bytes needed to append the 'value'
    uint32_t get_append_size_required(uint32_t value,uint32_t new_length){
        uint32_t new_min=std::min(_min_val,value);
        uint32_t new_max=std::max(_max_val,value);

        uint32_t diff=new_max-new_min;
        uint32_t b=(diff==0?0:32-__builtin_clz(diff));
        return MEDATA_OVERHEAD+4+for_compressed_size_bits(new_length,b);
    }

protected:
    uint8_t *_data;
    uint32_t _elm_count=0; // num of uint32 stored in array
    size_t _byte_length=0; // the actual num of bytes used(including overhead)
    size_t _byte_capacity; // the total num of bytes allocated

private:
    //for calculating the number of bits required after compression
    uint32_t _min_val=std::numeric_limits<uint32_t>::max();
    uint32_t _max_val=std::numeric_limits<uint32_t>::min();
};


} //namespace Index
} //namespace Skilo

#endif // COMPRESSABLEARRAY_H
