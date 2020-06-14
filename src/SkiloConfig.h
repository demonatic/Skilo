#ifndef SKILOCONFIG_H
#define SKILOCONFIG_H

#include "utility/ConfigReader.h"

namespace Skilo {

class SkiloConfig
{
public:
    SkiloConfig();

    template<typename Type>
    inline static Type get_conf(const std::string &field_path){
      return ConfigurationLoader::get_config(_config_path)->get<Type>(field_path);
    }

    const std::string &get_db_dir() const;
    const std::string &get_log_dir() const;

    void set_listen_port(const uint16_t port);
    void set_db_dir(const std::string &db_dir);

    uint16_t get_listen_port() const;
    void set_log_dir(const std::string &log_dir);

    const std::string &get_listen_address() const;

    const std::string &get_tokenizer_dict_dir() const;

private:
    std::string _database_dir;
    std::string _log_dir;

    uint16_t _listen_port;
    std::string _listen_address;

    std::string _tokenizer_dict_dir;

    static constexpr const char *_config_path="../skilo.conf";
};


} // namespace Skilo

#endif // SKILOCONFIG_H
