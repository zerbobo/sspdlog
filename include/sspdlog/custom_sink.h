#ifndef CUSTOM_SINK_H
#define CUSTOM_SINK_H

#include <spdlog/sinks/base_sink.h>

namespace sspdlog
{

class CustomSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
    CustomSink() = default;
    virtual ~CustomSink() = default;

protected:
    virtual void _sink_it(const spdlog::details::log_msg& msg) override = 0;
    virtual void flush() override = 0;

    CustomSink(const CustomSink &other) = delete;
    CustomSink &operator=(const CustomSink &other) = delete;
};

}
#endif