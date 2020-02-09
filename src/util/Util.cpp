#include "Util.h"


namespace Util {

NotFoundException::NotFoundException(const std::string &err)
{
    this->_msg="Not found error occured: "+err;
}

const char *NotFoundException::what() const noexcept
{
    return _msg.c_str();
}

InvalidFormatException::InvalidFormatException(const std::string &err)
{
    this->_msg="Invalid Format encountered: "+err;
}

const char *InvalidFormatException::what() const noexcept
{
    return _msg.c_str();
}

UnAuthorizedException::UnAuthorizedException(const std::string &err)
{
    this->_msg="Operation not authorized: "+err;
}

const char *UnAuthorizedException::what() const noexcept
{
    return _msg.c_str();
}

InternalServerException::InternalServerException(const std::string &err)
{
    this->_msg="Internal server error: "+err;
}

const char *InternalServerException::what() const noexcept
{
    return _msg.c_str();
}


}//namespace Util
