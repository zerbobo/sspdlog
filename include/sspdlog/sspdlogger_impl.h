#ifndef SSPDLOGGER_IMPL_H
#define SSPDLOGGER_IMPL_H

#include <set>
#include <cstring>
#ifdef __linux__
#include <unistd.h>
#include <libgen.h>
#endif

namespace sspdlog
{

inline Sspdlogger::Sspdlogger(const std::shared_ptr< SspdlogConfig > &conf)
{
    if (conf != nullptr) {
        _conf = conf;
        _config_source = DIRECT_PARAS;
    }
    else if (Sspdlogger::ExtConf() != nullptr) {
        _conf = Sspdlogger::ExtConf();
        _config_source = EXT_FUNC_SETTING;
    }
    else {
        _conf = std::make_shared< SspdlogConfig >();
        std::string filename = DEFAULT_CONF_FILE;
        if (Sspdlogger::ExtConfFile() != nullptr)
            filename = *Sspdlogger::ExtConfFile();
        if(_conf->ReadConfigFromSimpleFile(filename))  // there won't be any error if read file fails, just use defaults
            _config_source = EXT_FILE_SPECIFIED;
        else
            _config_source = DEFAULT_CONFIG;
    }
}

inline void Sspdlogger::Init()
{
    auto parse_names = [](const std::string &names) -> std::set< std::string > {
        std::set< std::string > result;
        if (names == "" || names.find_first_not_of(" \t") == std::string::npos)
            return result;
        std::string::size_type cur_p = 0;
        std::string s;
        while (true) {
            auto pos = names.find(',', cur_p);
            if (pos == std::string::npos)
                s = names.substr(cur_p);
            else
                s = names.substr(cur_p, pos - cur_p);
            if (result.find(s) == result.end())
                result.insert(s);
            if (pos == std::string::npos)
                break;
            cur_p = pos + 1;
        }
        return result;
    };

    auto get_level_enum = [](const std::string &level_name) -> spdlog::level::level_enum {
        for (int i = 0; i < spdlog::level::level_enum::off + 1; i++){
            if (!std::strcmp(spdlog::level::level_names[i], level_name.c_str()))
                return static_cast< spdlog::level::level_enum >(i);
        }
        return spdlog::level::debug;
    };

    // load all loggers
    auto conf = _conf;
    auto all_loggers = parse_names(conf->GetCurrentConfig(LOGGER_NAMES_KEY));
    all_loggers.insert(DEFAULT_LOGGER_NAME);
    for (auto &l : all_loggers){
        auto level = conf->GetCurrentConfig(std::string(LOGGER_LEVEL_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), l),
            std::string(LOGGER_LEVEL_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_LOGGER_NAME));
        auto format = conf->GetCurrentConfig(std::string(LOGGER_FORMAT_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), l),
            std::string(LOGGER_FORMAT_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_LOGGER_NAME));
        auto sink_names = conf->GetCurrentConfig(std::string(LOGGER_SINKS_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), l),
            std::string(LOGGER_SINKS_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_LOGGER_NAME));
        auto sinks = this->LoadSinks(parse_names(sink_names), conf);
        auto asyn = conf->GetCurrentConfig(std::string(LOGGER_ASYNC_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), l),
            std::string(LOGGER_ASYNC_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_LOGGER_NAME));

        std::shared_ptr< spdlog::logger > logger;
        if (asyn == "1"){
            const int one_m_size = 1024;
            logger = std::make_shared< spdlog::async_logger >(l, std::begin(sinks), std::end(sinks),
                one_m_size, spdlog::async_overflow_policy::block_retry, nullptr, std::chrono::milliseconds::zero());
        }
        else
            logger = std::make_shared< spdlog::logger >(l, std::begin(sinks), std::end(sinks));
        logger->set_level(get_level_enum(level));
        logger->set_pattern(format);
        spdlog::register_logger(logger);
    }
}

inline std::vector< spdlog::sink_ptr > Sspdlogger::LoadSinks(const std::set< std::string > &sink_names,
                                                             const std::shared_ptr< SspdlogConfig > &conf)
{
    // this function only parse two types of sinks: file, console, others should be parsed in inheritant class
    // file and console are shared by all loggers, ie. there are only one console sink, and one file sink if the file name is the same.
    std::vector< spdlog::sink_ptr > result;
    for (auto &s : sink_names) {
        auto name = s + SINK_KEY;
        if (name == CONSOLE_SINK_KEY) {
#ifdef _WIN32
#include "windows.h"
            class stdout_sink_mt_win : public spdlog::sinks::stdout_sink_mt
            {
            protected:
                void _sink_it(const spdlog::details::log_msg& msg) override
                {
                    std::string str_utf8(msg.formatted.data(), msg.formatted.size());
                    int len = ::MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, nullptr, 0);
                    wchar_t *wsz_gbk = new wchar_t[len + 1];
                    memset(wsz_gbk, 0, sizeof(wchar_t) * (len + 1));
                    ::MultiByteToWideChar(CP_UTF8, 0, str_utf8.c_str(), -1, wsz_gbk, len);

                    len = ::WideCharToMultiByte(CP_ACP, 0, wsz_gbk, -1, nullptr, 0, nullptr, nullptr);
                    char *sz_gbk = new char[len + 1];
                    memset(sz_gbk, 0, sizeof(char) * (len + 1));
                    ::WideCharToMultiByte(CP_ACP, 0, wsz_gbk, -1, sz_gbk, len, nullptr, nullptr);

                    _ostream.write(sz_gbk, len - 1);

                    delete[] sz_gbk;
                    delete[] wsz_gbk;

                    if (_force_flush)
                        _ostream.flush();
                }
            };
            static auto console_sink = std::make_shared< stdout_sink_mt_win >();
#else
            static auto console_sink = std::make_shared< spdlog::sinks::stdout_sink_mt >();
#endif
            result.push_back(console_sink);
        }
        else if (name.substr(name.size() - strlen(FILE_SINK_KEY)) == FILE_SINK_KEY) {
            static std::map< std::string, std::string > file_sink_names;
            static std::map< std::string, std::shared_ptr< spdlog::sinks::rotating_file_sink_mt > > file_sinks;
            std::shared_ptr< spdlog::sinks::rotating_file_sink_mt > file_sink;
            if (file_sink_names.find(name) == file_sink_names.end())
            {
                auto fn = conf->GetCurrentConfig(std::string(FILE_FULL_NAME_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), s),
                    std::string(FILE_FULL_NAME_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_FILE_SINK_NAME));
#ifdef __linux__
// http://man7.org/linux/man-pages/man2/readlink.2.html
// http://man7.org/linux/man-pages/man3/dirname.3.html
// http://man7.org/linux/man-pages/man2/chdir.2.html
                if(fn.length() > 1 && fn[0] == '$')
                {
                    char buf[256];
                    int len = readlink("/proc/self/exe", buf, 256);
                    if(len < 0 || len >= 256)
                        throw std::runtime_error("Buffer size to get exe path for log file is too small.");
                    buf[len] = '\0';
                    char *str = dirname(buf);
                    fn = std::string(str) + "/" + fn.substr(1);
                }
#endif
                file_sink_names[name] = fn;
                if (file_sinks.find(file_sink_names[name]) == file_sinks.end())
                {
                    auto filename = file_sink_names[name];
                    size_t max_size, max_num;
                    bool force_flush;
                    try{
                        max_size = std::stoi(conf->GetCurrentConfig(std::string(FILE_SIZE_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), s),
                            std::string(FILE_SIZE_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_FILE_SINK_NAME)));
                        max_num = std::stoi(conf->GetCurrentConfig(std::string(FILE_ROTATE_NUM_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), s),
                            std::string(FILE_ROTATE_NUM_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_FILE_SINK_NAME)));
                        force_flush = conf->GetCurrentConfig(std::string(FILE_FORCE_FLUSH_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), s),
                            std::string(FILE_FORCE_FLUSH_KEY).replace(0, std::strlen(SUBSTITUTE_KEY), DEFAULT_FILE_SINK_NAME)) != "0";
                    }
                    catch (const std::exception &){
                        throw SspdlogInitError("ERROR READ SSPDLOG CONFIG FOR FILE");
                    }
                    auto real_filename = filename;
                    int retry = 0;
                    while (true)
                    {
                        try
                        {
                            file_sink = std::make_shared< spdlog::sinks::rotating_file_sink_mt >(real_filename, "log",
                                max_size, max_num, force_flush);
                            break;
                        }
                        catch (const spdlog::spdlog_ex &e)
                        {
                            if (std::string(e.what()).find("Failed opening file") != std::string::npos && retry++ < 10)
                                real_filename = filename + ".tmp_random" + std::to_string(rand());
                            else
                                throw;
                        }
                    }
                    file_sinks[filename] = file_sink;
                }
                else
                    file_sink = file_sinks[file_sink_names[name]];
            }
            else
                file_sink = file_sinks[file_sink_names[name]];
            result.push_back(file_sink);
        }
    }
    return result;
}

};

#endif