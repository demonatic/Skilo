#ifndef NUMBER_H
#define NUMBER_H

#include <variant>
#include <stdint.h>

namespace Skilo {

struct number_t{
    std::variant<int64_t,double> num;

    number_t():num((int64_t)(0)){}

    number_t(const double f):num(f){

    }

    number_t(const int64_t i):num(i){

    }

    bool is_integer() const noexcept{
        return num.index()==0;
    }

    bool is_real() const noexcept{
        return num.index()==1;
    }

    number_t& operator=(const double f) noexcept{
        num=f;
        return *this;
    }

    number_t& operator=(const int64_t i) noexcept{
        num=i;
        return *this;
    }

    int64_t integer() const noexcept{
        if(is_integer()){
            return std::get<int64_t>(num);
        }
        return static_cast<int64_t>(std::get<double>(num));
    }

    double real() const noexcept{
        if(is_real()){
            return std::get<double>(num);
        }
        return std::get<int64_t>(num);
    }

    bool operator==(const number_t other) const noexcept{
        return this->num==other.num;
    }

    number_t operator*(const number_t &other) const noexcept{
        if(this->is_integer()&&other.is_integer()){
            return number_t(this->integer()*other.integer());
        }
        return number_t(this->real()*other.real());
    }

    bool operator<(const number_t &other) const noexcept{
        if(this->is_integer()&&other.is_integer()){
            return std::get<int64_t>(num)<std::get<int64_t>(other.num);
        }
        return real()<other.real();
    }

    bool operator>(const number_t &other) const noexcept{
        return !this->operator<(other);
    }

    number_t operator-() noexcept{
        if(is_integer()){
            this->num=-std::get<int64_t>(num);
        }
        else{
            this->num=-std::get<double>(num);
        }
        return *this;
    }
};

} //namespace Skilo
#endif // NUMBER_H
