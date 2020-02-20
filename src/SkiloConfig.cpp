#include "SkiloConfig.h"

namespace Skilo {

Skilo::SkiloConfig::SkiloConfig()
{
    _log_dir=get_conf<std::string>("primary.log_dir");
    _database_dir=get_conf<std::string>("primary.db_dir");
    _listen_port=get_conf<size_t>("network.port");
    _listen_address=get_conf<std::string>("network.address");
    _tokenizer_dict_dir=get_conf<std::string>("extensions.dict_dir");
    if(_log_dir.empty()||_database_dir.empty()||!_listen_port||_listen_address.empty()||_tokenizer_dict_dir.empty()){
        perror("load config failed");
        exit(-1);
    }
}

const std::string &SkiloConfig::get_db_dir() const
{
    return _database_dir;
}

const std::string &SkiloConfig::get_log_dir() const
{
    return _log_dir;
}

uint16_t SkiloConfig::get_listen_port() const
{
    return _listen_port;
}

const std::string &SkiloConfig::get_listen_address() const
{
    return _listen_address;
}

const std::string &SkiloConfig::get_tokenizer_dict_dir() const
{
    return _tokenizer_dict_dir;
}

} // namespace Skilo
