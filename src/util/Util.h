#ifndef UTIL_H
#define UTIL_H

#include <stdexcept>

namespace Util {

    class SkiloException:public std::exception{
    public:
        SkiloException()=default;
        virtual const char *what() const noexcept override;
    protected:
        std::string _msg;
    };

    class InvalidFormatException:public SkiloException{
    public:
        InvalidFormatException(const std::string &err);
    };

    class NotFoundException:public SkiloException{
    public:
        NotFoundException(const std::string &err);
    };

    class ConflictException:public SkiloException{
    public:
        ConflictException(const std::string &err);
    };

    class InternalServerException:public SkiloException{
    public:
        InternalServerException(const std::string &err);
    };

    class UnAuthorizedException:public SkiloException{
    public:
        UnAuthorizedException(const std::string &err);
    };

}//namespace Util

#endif // UTIL_H
