## Introduction

*Super SPD Log*

A simple head-only log system.

You can use it in your project, like:

```c++
#include <sspdlog/sspdlog.h>
SSPD_LOG_INFO << "this is a log message";
SSPD_LOG_WARNING_F("this is a log {}", "message");
```

which will automatically log message to console and files using a default configuration.


## Requirement

*spdlog* https://github.com/gabime/spdlog

(As spdlog is a head-only third party lib, all its codes are here too)

Some native modifications have been made based on the original source code.


## Build

It is tested on Ubuntu 1604, with gcc 5.4.0; It also support windows, but the tests are not sufficient yet.

To build on ubuntu to run tests and examples, you have to install gmock first https://github.com/google/googletest

Then run `tools/build.sh` to build automatically, it will also install sspdlog into your system, and produce a `.deb` package using `checkinstall`.


## About Configuration

1) Use a file named "sspdlog.conf"(default) to config sspdlog, which should be in the exe working
dir. (If there are no any config files found, the system will use a default config
which is defined in `include/sspdlog_config.h`)

2) A config file(default) can be like this:
```
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
```
(supported format of log message can refer to: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)

3) You can also use
```
void set_custom_sspdlog_config(std::shared_ptr< std::map< std::string, std::string > > conf, bool clear_old_config = false);
void set_custom_sspdlog_config_file(const std::string &filename);
```
to customize the log configurations.
*WARNING*: Configuration should be done before any usage of log method like `SSPD_LOG_INFO<<...`, or
it will throw an exception.

The given map can be incomplete, like this:
```
{
"custom_logger_names":"user_logger",
"user_logger_async":1,
"user_logger_sinks":"user_jack_file",
"user_jack_file_full_name":"jack.txt"
}
```
Other keywords will use default values. All keywords are:
```
// origianl keywords
custom_logger_names, root_logger_async, root_logger_level, root_logger_format, root_logger_sinks, console_sink,
file_sink, file_full_name, file_size, file_rotate_num, file_force_flush
```
```
// user configed keywords
*_async, *_level, *_format, *_sinks, //(* is the name defined through custom_logger_names)
*file_sink, *file_full_name, *file_size, *file_rotate_num, *file_force_flush, //(* is the name defined through *_sinks)
```

## Other Extensions

It supports modifications to the lib to create more user defined behavior.

All you need to do are:

1) inherit class `CustomSink` in `include/custom_sink.h`, to rewrite `_sink_it` and `flush`,
producing user defined sinks.

2) inherit class `Sspdlogger` in `include/sspdlogger.h` to rewrite `LoadSinks`, configuring
your new sinks loading.

3) Config your logger using `sspdlog::SspdlogConfig` class structure.

4) Create your logger at the very beginning of your app like:
`sspdlog::Sspdlogger::Instance(std::make_shared</*YOUR_LOGGER_CLASS*/>(/*PARAS*/));`
