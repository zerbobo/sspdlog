//
// Example4:
//      1) Just log directly using all default configs
//      2) print the used log config
//

#include <sspdlog/sspdlog.h>

int main()
{
    std::cout << "Example4 start..." << std::endl;

    SSPD_LOG_INFO << "This is a log message";

    std::cout << "Example4 run over." << std::endl;

    auto src = SSPDLOGGER_INSTANCE->GetLogConfigSource();
    std::cout << "Used config from: " << (src == sspdlog::DIRECT_PARAS ? "DIRECT_PARAS" :
                                          (src == sspdlog::EXT_FUNC_SETTING ? "EXT_FUNC_SETTING" :
                                           (src == sspdlog::EXT_FILE_SPECIFIED ? "EXT_FILE_SPECIFIED" :
                                            (src == sspdlog::DEFAULT_CONFIG ? "DEFAULT_CONFIG" : "UNKNOWN")))) << std::endl;
    SSPDLOGGER_INSTANCE->GetLogConfig()->PrintConfig2Console();
    return 0;
}