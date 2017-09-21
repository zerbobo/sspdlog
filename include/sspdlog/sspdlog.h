#ifndef SSPDLOG_H
#define SSPDLOG_H

#include "sspdlogger.h"
#include "sspdlog_config.h"

namespace sspdlog{

// config the nlog from a dict or a file, it will be mixed with the default config.
void set_custom_sspdlog_config(const std::shared_ptr< std::map< std::string, std::string > > &conf, bool clear_old_config = false);
void set_custom_sspdlog_config_file(const std::string &filename);

void close_colored_log(bool if_colored = false);

}

#define SSPDLOGGER_INSTANCE sspdlog::Sspdlogger::Instance()
#define SSPD_LOG_LINE_INFO spdlog::details::add_msg(__FILE__, __FUNCTION__, __LINE__)

#define SSPD_LOG_DEBUG_F(...)    SSPDLOGGER_INSTANCE->GetSpdLogger(sspdlog::DEFAULT_LOGGER_NAME)->debug(SSPD_LOG_LINE_INFO, __VA_ARGS__)
#define SSPD_LOG_INFO_F(...)     SSPDLOGGER_INSTANCE->GetSpdLogger(sspdlog::DEFAULT_LOGGER_NAME)->info(SSPD_LOG_LINE_INFO, __VA_ARGS__)
#define SSPD_LOG_WARNING_F(...)  SSPDLOGGER_INSTANCE->GetSpdLogger(sspdlog::DEFAULT_LOGGER_NAME)->warn(SSPD_LOG_LINE_INFO, __VA_ARGS__)
#define SSPD_LOG_ERROR_F(...)    SSPDLOGGER_INSTANCE->GetSpdLogger(sspdlog::DEFAULT_LOGGER_NAME)->error(SSPD_LOG_LINE_INFO, __VA_ARGS__)
#define SSPD_LOG_CRITICAL_F(...) SSPDLOGGER_INSTANCE->GetSpdLogger(sspdlog::DEFAULT_LOGGER_NAME)->critical(SSPD_LOG_LINE_INFO, __VA_ARGS__)

#define SSPD_LOG_DEBUG       SSPD_LOG_DEBUG_F("")
#define SSPD_LOG_INFO        SSPD_LOG_INFO_F("")
#define SSPD_LOG_WARNING     SSPD_LOG_WARNING_F("")
#define SSPD_LOG_ERROR       SSPD_LOG_ERROR_F("")
#define SSPD_LOG_CRITICAL    SSPD_LOG_CRITICAL_F("")

// use like this:
// SSPD_LOG_INFO << "THIS IS A LOG MESSAGES FROM" << var;
// SSPD_LOG_INFO_F("this is a log message from {} and {}", var, var2);

/* A method to make log macros empty:
struct EmptyLogger
{
    template<class T>
    EmptyLogger &operator<<(const T&){ return *this; }
};
#define SSPD_LOG_DEBUG_F(...)
#define SSPD_LOG_INFO_F(...)
#define SSPD_LOG_WARNING_F(...)
#define SSPD_LOG_ERROR_F(...)
#define SSPD_LOG_CRITICAL_F(...)
#define SSPD_LOG_DEBUG           EmptyLogger()
#define SSPD_LOG_INFO            SSPD_LOG_DEBUG
#define SSPD_LOG_WARNING         SSPD_LOG_DEBUG
#define SSPD_LOG_ERROR           SSPD_LOG_DEBUG
#define SSPD_LOG_CRITICAL        SSPD_LOG_DEBUG
*/

#include "sspdlog_impl.h"

#endif
