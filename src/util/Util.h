#ifndef UTIL_H
#define UTIL_H

#include <stdexcept>

namespace Util {

    class InvalidFormatException:public std::exception{
    public:
        InvalidFormatException(const std::string &err);
        virtual const char *what() const noexcept override;
    private:
        std::string _msg;
    };

    class NotFoundException:public std::exception{
    public:
        NotFoundException(const std::string &err);
        virtual const char *what() const noexcept override;
    private:
        std::string _msg;
    };

    class InternalServerException:public std::exception{
    public:
        InternalServerException(const std::string &err);
        virtual const char *what() const noexcept override;
    private:
        std::string _msg;
    };

    class UnAuthorizedException:public std::exception{
    public:
        UnAuthorizedException(const std::string &err);
        virtual const char *what() const noexcept override;
    private:
        std::string _msg;
    };

}//namespace Util

#endif // UTIL_H
