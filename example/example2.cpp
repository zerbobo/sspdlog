//
// Example2:
//      1) setting config using an external file, config more logger other than the root logger
//      2) using different loggers as needed
//      3) print the used log config
//

#include <sspdlog/sspdlog.h>

int main()
{
    std::cout << "Example2 start..." << std::endl;
    sspdlog::set_custom_sspdlog_config_file("example2_config.txt");

    // 1.
    SSPD_LOG_INFO << "A first log message from " << "root";

    // 2.
    SSPDLOGGER_INSTANCE->GetSpdLogger("example_log1")->info(SSPD_LOG_LINE_INFO) << "A second log message from example_log1"; // too low level, so actually no log
    SSPDLOGGER_INSTANCE->GetSpdLogger("example_log1")->warn(SSPD_LOG_LINE_INFO) << "A third log message from example_log1";

    // 3.
    SSPDLOGGER_INSTANCE->GetSpdLogger("example_log2")->info(SSPD_LOG_LINE_INFO) << "A fourth log message from example_log2"; // same log level setting as the root, so log ok

    std::cout << "Example2 run over." << std::endl;

    auto src = SSPDLOGGER_INSTANCE->GetLogConfigSource();
    std::cout << "Used config from: " << (src == sspdlog::DIRECT_PARAS ? "DIRECT_PARAS" :
                                          (src == sspdlog::EXT_FUNC_SETTING ? "EXT_FUNC_SETTING" :
                                           (src == sspdlog::EXT_FILE_SPECIFIED ? "EXT_FILE_SPECIFIED" :
                                            (src == sspdlog::DEFAULT_CONFIG ? "DEFAULT_CONFIG" : "UNKNOWN")))) << std::endl;
    SSPDLOGGER_INSTANCE->GetLogConfig()->PrintConfig2Console();
    return 0;
}