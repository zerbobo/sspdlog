#ifndef SSPDLOGGER_H
#define SSPDLOGGER_H

#include <mutex>
#include <memory>
#include <set>
#include <chrono>
#include "sspdlog_config.h"
#include "sspdlog_errors.h"
#include <spdlog/spdlog.h>

namespace sspdlog
{

enum LOGGER_CONFIG_SOURCE
{
    DEFAULT_CONFIG = 0,
    DIRECT_PARAS = 1,
    EXT_FUNC_SETTING = 2,
    EXT_FILE_SPECIFIED = 3,
    UNKNOWN = 4
};
        
class Sspdlogger
{
private:
    struct _SingletonInfo
    {
        std::mutex _sspdlog_init_mtx, _sspdlog_conf_mtx, _sspdlog_file_mtx;
        std::shared_ptr< Sspdlogger > _sspdlogger = nullptr;
        std::shared_ptr< SspdlogConfig > _ext_conf = nullptr;
        std::shared_ptr< std::string > _filename = nullptr;
    };
    static std::shared_ptr< _SingletonInfo > _SingletonControlledInfo()
    {
        static std::shared_ptr< _SingletonInfo > _info = std::make_shared< _SingletonInfo >();
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while (!_info)
        {
            if (std::chrono::duration_cast< std::chrono::seconds >(std::chrono::steady_clock::now() - start).count() > 10)
                throw SspdlogInitError("Sspdlog init error, mostly because of mutex error between threads.");
        }
        return _info;
    };

public:
    static std::shared_ptr< Sspdlogger > Instance(std::shared_ptr< Sspdlogger > logger = nullptr)
    {
        auto info = Sspdlogger::_SingletonControlledInfo();
        std::lock_guard< std::mutex > lock(info->_sspdlog_init_mtx);
        if (logger && info->_sspdlogger)
            throw SspdlogInitError("Sspdlog is already inited before creating an external Sspdlog object.");
        if (info->_sspdlogger == nullptr){
            if (logger)
                info->_sspdlogger = logger;
            else 
                info->_sspdlogger = std::shared_ptr< Sspdlogger >(new Sspdlogger());
            info->_sspdlogger->Init();
        }
        return info->_sspdlogger;
    };

    static std::shared_ptr< SspdlogConfig > ExtConf(const std::shared_ptr< std::map< std::string, std::string > > &conf = nullptr,
                                                    bool clear_old_config = false)
    {
        auto info = Sspdlogger::_SingletonControlledInfo();
        std::lock_guard< std::mutex > lock(info->_sspdlog_conf_mtx);
        if (conf) {
            if (info->_sspdlogger)
                throw SspdlogInitError("Sspdlog is already inited before passing in external SspdlogConfig.");
            if (clear_old_config || info->_ext_conf == nullptr){
                info->_ext_conf = std::make_shared< SspdlogConfig >();
                info->_ext_conf->SetConfig(*conf);
            }
            else
                info->_ext_conf->UpdateConfig(*conf);
        }
        return info->_ext_conf;
    };

    static std::shared_ptr<std::string> ExtConfFile(const std::shared_ptr< std::string > &filename = nullptr)
    {
        auto info = Sspdlogger::_SingletonControlledInfo();
        std::lock_guard< std::mutex > lock(info->_sspdlog_file_mtx);
        if (filename) {
            if (info->_sspdlogger)
                throw SspdlogInitError("Sspdlog is already inited before passing in external sspdlog config file.");
            info->_filename = std::make_shared< std::string >(*filename);
        }
        return info->_filename;
    };
    
    virtual ~Sspdlogger() = default;

    std::shared_ptr< SspdlogConfig > GetLogConfig() const {
        return _conf;
    };

    LOGGER_CONFIG_SOURCE GetLogConfigSource() const {
        return _config_source;
    };

    std::shared_ptr< spdlog::logger > GetSpdLogger(const std::string &name,
                                                   const std::string &default_one = DEFAULT_LOGGER_NAME) const {
        auto l = spdlog::get(name);
        if (l)
            return l;
        return spdlog::get(default_one);
    };

protected:
    Sspdlogger(const std::shared_ptr< SspdlogConfig > &conf = nullptr);
    void Init();

    virtual std::vector< spdlog::sink_ptr > LoadSinks(const std::set< std::string > &sink_names,
                                                      const std::shared_ptr< SspdlogConfig > &conf);

    Sspdlogger(const Sspdlogger &) = delete;
    const Sspdlogger &operator=(const Sspdlogger &) = delete;

    std::shared_ptr< SspdlogConfig > _conf;
    LOGGER_CONFIG_SOURCE _config_source = UNKNOWN;
};

}

#include "sspdlogger_impl.h"

#endif