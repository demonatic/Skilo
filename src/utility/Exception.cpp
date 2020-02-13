#include "Exception.h"

namespace Skilo {

const char *SkiloException::what() const noexcept
{
    return _msg.c_str();
}

NotFoundException::NotFoundException(const std::string &err)
{
    this->_msg="Resource Not Found: "+err;
}

InvalidFormatException::InvalidFormatException(const std::string &err)
{
    this->_msg="Invalid Format Encountered: "+err;
}

UnAuthorizedException::UnAuthorizedException(const std::string &err)
{
    this->_msg="Operation Not Authorized: "+err;
}

InternalServerException::InternalServerException(const std::string &err)
{
    this->_msg="Internal Server Error: "+err;
}

ConflictException::ConflictException(const std::string &err)
{
    this->_msg="Conflict Resource: "+err;
}

} //namespace Skilo
