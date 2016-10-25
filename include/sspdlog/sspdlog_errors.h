#ifndef SSPDLOG_ERRORS_H
#define SSPDLOG_ERRORS_H

#include <exception>
#include <string>

namespace sspdlog{

class SspdlogException : public std::exception
{
public:
    virtual ~SspdlogException() = default;
#ifdef _WIN32
    virtual const char* what() const { return "SspdlogException"; };
#else
    virtual const char* what() const noexcept (true) { return "SspdlogException"; };
#endif
};

class SspdlogInitError : public SspdlogException
{
    std::string _description = "";
public:
    SspdlogInitError(const std::string &description = "SspdlogInitError") :_description(description){};
    virtual ~SspdlogInitError() = default;
#ifdef _WIN32
    virtual const char* what() const { return _description.c_str(); };
#else
    virtual const char* what() const noexcept (true) { return _description.c_str(); };
#endif
};

}

#endif