#ifndef SSPDLOG_IMPL_H
#define SSPDLOG_IMPL_H

namespace sspdlog {

inline void set_custom_sspdlog_config(const std::shared_ptr< std::map< std::string, std::string > > &conf, bool clear_old_config)
{
    Sspdlogger::ExtConf(conf, clear_old_config);
}

inline void set_custom_sspdlog_config_file(const std::string &filename)
{
    Sspdlogger::ExtConfFile(std::make_shared< std::string >(filename));
}

inline void close_colored_log(bool if_colored)
{
    spdlog::close_colored_log(if_colored);
}

}

#endif