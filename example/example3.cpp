//
// Example3:
//      1) setting config using a map
//      2) using the default root log
//      3) print the used log config
//

#include <sspdlog/sspdlog.h>

int main()
{
    std::cout << "Example3 start..." << std::endl;
    auto conf = std::make_shared< std::map< std::string, std::string > >();
    (*conf)["root_logger_format"] = "[%Y-%m-%d %H:%M:%S.%e]-[%l]- %v (#f ##l #F)[Using config from a map]";
    (*conf)["file_full_name"] = "./example3log";
    sspdlog::set_custom_sspdlog_config(conf);

    SSPD_LOG_INFO << "This is a log message";

    std::cout << "Example3 run over." << std::endl;

    auto src = SSPDLOGGER_INSTANCE->GetLogConfigSource();
    std::cout << "Used config from: " << (src == sspdlog::DIRECT_PARAS ? "DIRECT_PARAS" :
                                          (src == sspdlog::EXT_FUNC_SETTING ? "EXT_FUNC_SETTING" :
                                           (src == sspdlog::EXT_FILE_SPECIFIED ? "EXT_FILE_SPECIFIED" :
                                            (src == sspdlog::DEFAULT_CONFIG ? "DEFAULT_CONFIG" : "UNKNOWN")))) << std::endl;
    SSPDLOGGER_INSTANCE->GetLogConfig()->PrintConfig2Console();
    return 0;
}