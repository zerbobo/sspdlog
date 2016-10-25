#include <fstream>
#include <gtest/gtest.h>
#include <sspdlog/sspdlog.h>

class SspdBasicTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SspdBasicTest, SetConfigUsingMap) {
    auto con1 = std::make_shared< std::map< std::string, std::string > >();
    (*con1)["custom_logger_names"] = "simple_log";
    sspdlog::set_custom_sspdlog_config(con1);
    EXPECT_STREQ("simple_log", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("custom_logger_names").c_str());
    EXPECT_STRNE("", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("root_logger_format").c_str());
    EXPECT_STREQ("debug", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("root_logger_level").c_str());
    auto con2 = std::make_shared< std::map< std::string, std::string > >();
    (*con2)["root_logger_level"] = "warning";
    sspdlog::set_custom_sspdlog_config(con2, true);
    EXPECT_STREQ("", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("custom_logger_names").c_str());
    EXPECT_STRNE("", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("root_logger_format").c_str());
    EXPECT_STREQ("warning", sspdlog::Sspdlogger::ExtConf()->GetCurrentConfig("root_logger_level").c_str());
}

TEST_F(SspdBasicTest, LogNoExceptions) {
    auto con = std::make_shared< std::map< std::string, std::string > >(sspdlog::CONFIG_MAP_DEFAULT);
    (*con)["root_logger_format"] = "[%Y - %m - %d %H:%M : %S.%e](#F) - [%l] - %v(#f ##l)";
    sspdlog::set_custom_sspdlog_config(con, true);

    EXPECT_NO_THROW(SSPD_LOG_DEBUG << "log " << 1 << ": using \"LOG_DEBUG\" correct");
    EXPECT_NO_THROW(SSPD_LOG_INFO << "log " << 2 << ": using \"LOG_INFO\" correct");
    EXPECT_NO_THROW(SSPD_LOG_WARNING << "log " << 3 << ": using \"LOG_WARNING\" correct");
    EXPECT_NO_THROW(SSPD_LOG_ERROR << "log " << 4 << ": using \"LOG_ERROR\" correct");
    EXPECT_NO_THROW(SSPD_LOG_CRITICAL << "log " << 5 << ": using \"LOG_CRITICAL\" correct");
    EXPECT_NO_THROW(SSPD_LOG_DEBUG_F("log {} using \"LOG_DEBUG_F\" correct", 6));
    EXPECT_NO_THROW(SSPD_LOG_INFO_F("log {} using \"LOG_INFO_F\" correct", 7));
    EXPECT_NO_THROW(SSPD_LOG_WARNING_F("log {} using \"LOG_WARNING_F\" correct", 8));
    EXPECT_NO_THROW(SSPD_LOG_ERROR_F("log {} using \"LOG_ERROR_F\" correct", 9));
    EXPECT_NO_THROW(SSPD_LOG_CRITICAL_F("log {} using \"LOG_CRITICAL_F\" correct", 10));

    SSPD_LOG_INFO << "Test log info for LogFileAsExpected";
    std::ifstream f("defaultLog.log");
    std::string s, tmp;
    while (!f.eof())
    {
        std::getline(f, tmp);
        if (tmp.length() > 0)
            s = tmp;
    }
    std::string target = "- [info] - Test log info for LogFileAsExpected(sspdlog_basic_test.cpp #43)";
    EXPECT_STREQ(target.c_str(), s.substr(s.length() - target.length()).c_str());
}

TEST_F(SspdBasicTest, SettingShouldBeforeLog) {
    SSPD_LOG_WARNING_F("Test a warning log message.");
    auto con = std::make_shared< std::map< std::string, std::string > >();
    EXPECT_THROW(sspdlog::set_custom_sspdlog_config(con), sspdlog::SspdlogInitError);
}   
