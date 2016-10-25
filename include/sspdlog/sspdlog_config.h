#ifndef SSPDLOG_CONFIG_H
#define SSPDLOG_CONFIG_H

#include <map>
#include <vector>
#include <string>
#include "sspdlog_errors.h"

namespace sspdlog
{

const char SUBSTITUTE_KEY[] = "*";
const char LOGGER_NAMES_KEY[] = "custom_logger_names";
const char LOGGER_ASYNC_KEY[] = "*_async";
const char LOGGER_LEVEL_KEY[] = "*_level";
const char LOGGER_FORMAT_KEY[] = "*_format";
const char LOGGER_SINKS_KEY[] = "*_sinks";

const char SINK_KEY[] = "_sink";
const char CONSOLE_SINK_KEY[] = "console_sink";
const char FILE_SINK_KEY[] = "file_sink";
const char FILE_FULL_NAME_KEY[] = "*_full_name";
const char FILE_SIZE_KEY[] = "*_size";
const char FILE_ROTATE_NUM_KEY[] = "*_rotate_num";
const char FILE_FORCE_FLUSH_KEY[] = "*_force_flush";

const char LEVEL_NAME_DEBUG[] = "debug";
const char LEVEL_NAME_INFO[] = "info";
const char LEVEL_NAME_WARNING[] = "warning";
const char LEVEL_NAME_ERROR[] = "error";
const char LEVEL_NAME_FATAL[] = "critical";

const char DEFAULT_CONF_FILE[] = "sspdlog.conf";
const char DEFAULT_LOGGER_NAME[] = "root_logger";
const char DEFAULT_FILE_SINK_NAME[] = "file";
const std::map< std::string, std::string > CONFIG_MAP_DEFAULT = {
    { LOGGER_NAMES_KEY, "" },
    { std::string(DEFAULT_LOGGER_NAME) + "_async", "0" },
    { std::string(DEFAULT_LOGGER_NAME) + "_level", LEVEL_NAME_DEBUG },
    { std::string(DEFAULT_LOGGER_NAME) + "_format", "[%Y-%m-%d %H:%M:%S.%e]-[%l]- %v (#f ##l #F)" },
    { std::string(DEFAULT_LOGGER_NAME) + "_sinks", "console,file" },
    { CONSOLE_SINK_KEY, "Console" },
    { std::string(DEFAULT_FILE_SINK_NAME) + "_sink", "RotateFile" },
    { std::string(DEFAULT_FILE_SINK_NAME) + "_full_name", "./defaultLog" },
    { std::string(DEFAULT_FILE_SINK_NAME) + "_size", "1048576" },
    { std::string(DEFAULT_FILE_SINK_NAME) + "_rotate_num", "3" },
    { std::string(DEFAULT_FILE_SINK_NAME) + "_force_flush", "1" }
};

class SspdlogConfig
{
public:
    virtual ~SspdlogConfig() = default;

    std::string GetCurrentConfig(const std::string &key, const std::string &default_key="") const
    {
        try{
            return _config_map.at(key);
        }
        catch (const std::out_of_range &){
            if (default_key != "")
                return _config_map.at(default_key);
            else
                throw SspdlogInitError("ERROR GET CONFIG FROM CURRENT SSPDLOG CONFIG FOR KEY:" + key);
        } 
    };
    // add or modify configs described in 'conf', leave others (no deletes)
    void UpdateConfig(const std::map< std::string, std::string > &conf);
    // replace configs with values from 'conf', but don't delete values which are default ones
    void SetConfig(const std::map< std::string, std::string > &conf);

    // file like this (the result is as the default one):
    /*
    custom_logger_names =   ""
    root_logger_async   =   0
    root_logger_level   =   "debug"
    root_logger_format  =   "[%Y-%m-%d %H:%M:%S.%e]-[%l]- %v (#f ##l #F)"
    root_logger_sinks   =   "console,file"
    console_sink        =   "Console"
    file_sink           =   "RotateFile"
    file_full_name      =   "./defaultLog"
    file_size           =   1048576
    file_rotate_num     =   3
    file_force_flush    =   1
    */
    bool ReadConfigFromSimpleFile(const std::string &filename);

    void PrintConfig2Console();

private:    
    std::map< std::string, std::string > _config_map = CONFIG_MAP_DEFAULT;
};

}

#include "sspdlog_config_impl.h"

#endif