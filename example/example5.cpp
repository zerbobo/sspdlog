//
// Example5:
//      1) setting config using an external file
//      2) using the default root log
//      3) print the used log config
//

#include <sspdlog/sspdlog.h>

int main()
{
    std::cout << "Example5 start..." << std::endl;
    sspdlog::set_custom_sspdlog_config_file("example5_config.txt");

    // 1.
    std::string user = "xiaoming";
    SSPD_LOG_INFO << "A first log message from " << user;

    // 2.
    int num = 2;
    SSPD_LOG_WARNING_F("A second log message is called {}", num);

    // 3.
    SSPD_LOG_ERROR_F("A third log message is '{}' {}", "hello world", 123);

    // 4.
    double d = 1234.5678;
    SSPD_LOG_CRITICAL << "A fourth log message is double number:" << d;

    // 5.
    sspdlog::close_colored_log();
    SSPD_LOG_WARNING << "A fifth log of warning without a color";

    // 6.
    SSPD_LOG_INFO << "LOG ALL OVER";

    std::cout << "Example5 run over." << std::endl;

    auto src = SSPDLOGGER_INSTANCE->GetLogConfigSource();
    std::cout << "Used config from: " << (src == sspdlog::DIRECT_PARAS ? "DIRECT_PARAS" :
                                          (src == sspdlog::EXT_FUNC_SETTING ? "EXT_FUNC_SETTING" :
                                           (src == sspdlog::EXT_FILE_SPECIFIED ? "EXT_FILE_SPECIFIED" :
                                            (src == sspdlog::DEFAULT_CONFIG ? "DEFAULT_CONFIG" : "UNKNOWN")))) << std::endl;
    SSPDLOGGER_INSTANCE->GetLogConfig()->PrintConfig2Console();
    return 0;
}