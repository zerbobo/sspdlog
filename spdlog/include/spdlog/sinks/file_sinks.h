/*************************************************************************/
/* spdlog - an extremely fast and easy to use c++11 logging library.     */
/* Copyright (c) 2014 Gabi Melman.                                       */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#pragma once

#include <mutex>
#include "base_sink.h"
#include "../details/null_mutex.h"
#include "../details/file_helper.h"
#include "../details/format.h"
#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <regex>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include "../details/dirent.h"
#else
#include <dirent.h>
#endif

namespace spdlog
{
namespace sinks
{
/*
* Trivial file sink with single file as target
*/
template<class Mutex>
class simple_file_sink : public base_sink < Mutex >
{
public:
    explicit simple_file_sink(const std::string &filename,
                              bool force_flush = false) :
        _file_helper(force_flush)
    {
        _file_helper.open(filename);
    }
    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _file_helper.write(msg);
    }
private:
    details::file_helper _file_helper;
};

typedef simple_file_sink<std::mutex> simple_file_sink_mt;
typedef simple_file_sink<details::null_mutex> simple_file_sink_st;

/*
* Rotating file sink based on size
*/
template<class Mutex>
class rotating_file_sink : public base_sink < Mutex >
{
public:
    rotating_file_sink(const std::string &base_filename, const std::string &extension,
                       std::size_t max_size, std::size_t max_files,
                       bool force_flush = false) :
        _base_filename(base_filename),
        _extension(extension),
        _max_size(max_size),
        _max_files(max_files),
        _current_size(0),
        _file_helper(force_flush)
    {
        _file_helper.open(calc_filename(_base_filename, 0, _extension));
    }

    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        _current_size += msg.formatted.size();
        if (_current_size > _max_size)
        {
            _rotate();
            _current_size = msg.formatted.size();
        }
        _file_helper.write(msg);
    }

private:
    static std::string calc_filename(const std::string& filename, std::size_t index, const std::string& extension)
    {
        fmt::MemoryWriter w;
        if (index)
            w.write("{}.{}.{}", filename, extension, index);
        else
            w.write("{}.{}", filename, extension);
        return w.str();
    }

    // Rotate files:
    // log.txt -> log.txt.1
    // log.txt.1 -> log.txt.2
    // log.txt.2 -> log.txt.3
    // log.txt.3 -> delete

    void _rotate()
    {
        _file_helper.close();
        for (auto i = _max_files; i > 0; --i)
        {
            std::string src = calc_filename(_base_filename, i - 1, _extension);
            std::string target = calc_filename(_base_filename, i, _extension);

            if (details::file_helper::file_exists(target))
            {
                if (std::remove(target.c_str()) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + target);
                }
            }
            if (details::file_helper::file_exists(src) && std::rename(src.c_str(), target.c_str()))
            {
                throw spdlog_ex("rotating_file_sink: failed renaming " + src + " to " + target);
            }
        }
        _file_helper.reopen(true);
    }
    std::string _base_filename;
    std::string _extension;
    std::size_t _max_size;
    std::size_t _max_files;
    std::size_t _current_size;
    details::file_helper _file_helper;
};

typedef rotating_file_sink<std::mutex> rotating_file_sink_mt;
typedef rotating_file_sink<details::null_mutex>rotating_file_sink_st;

/*
* Rotating file sink based on date. rotates at midnight
*/
template<class Mutex>
class daily_file_sink :public base_sink < Mutex >
{
public:
    //create daily file sink which rotates on given time
    daily_file_sink(
        const std::string& base_filename,
        const std::string& extension,
        int rotation_hour,
        int rotation_minute,
        std::size_t max_files,
        bool force_flush = false) : _base_filename(base_filename),
        _extension(extension),
        _rotation_h(rotation_hour),
        _rotation_m(rotation_minute),
        _max_files(max_files),
        _file_helper(force_flush)
    {
        if (rotation_hour < 0 || rotation_hour > 23 || rotation_minute < 0 || rotation_minute > 59)
            throw spdlog_ex("daily_file_sink: Invalid rotation time in ctor");
        auto file_name = calc_filename(_base_filename, 0, _extension);
#ifdef __linux__
        if (details::file_helper::file_exists(file_name))
        {
            struct stat st;
            stat(file_name.c_str(), &st);
            _rotation_tp = _next_rotation_tp(&(st.st_mtime));
        }
        else
            _rotation_tp = _next_rotation_tp();
#else
        _rotation_tp = _next_rotation_tp();
#endif
        _file_helper.open(file_name);
    }

    void flush() override
    {
        _file_helper.flush();
    }

protected:
    void _sink_it(const details::log_msg& msg) override
    {
        if (std::chrono::system_clock::now() >= _rotation_tp)
        {
            _rotate();
            _rotation_tp = _next_rotation_tp();
        }
        _file_helper.write(msg);
    }

private:
    std::chrono::system_clock::time_point _next_rotation_tp(std::time_t *mt = nullptr)
    {
        using namespace std::chrono;
        time_t tnow;
        if(mt)
        {
            tnow = *mt;
        }
        else
        {           
            auto now = system_clock::now(); 
            tnow = std::chrono::system_clock::to_time_t(now);
        }
        auto mt_point = std::chrono::system_clock::from_time_t(tnow);
        tm date = spdlog::details::os::localtime(tnow);
        date.tm_hour = _rotation_h;
        date.tm_min = _rotation_m;
        date.tm_sec = 0;
        auto rotation_time = std::chrono::system_clock::from_time_t(std::mktime(&date));
        if (rotation_time > mt_point)
            return rotation_time;
        else
            return system_clock::time_point(rotation_time + hours(24));
    }

    std::string calc_filename(const std::string& basename, std::size_t index, const std::string& extension)
    {
        using namespace std::chrono;
        fmt::MemoryWriter w;
        if (index)
        {
            auto tp = system_clock::time_point(_rotation_tp - hours(24));
            time_t tp_timet = std::chrono::system_clock::to_time_t(tp);
            std::tm tm = spdlog::details::os::localtime(tp_timet);
            w.write("{}.{}.{:04d}{:02d}{:02d}.{:02d}{:02d}", basename, extension, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
        }
        else
            w.write("{}.{}", basename, extension);
        return w.str();
    }

    void _rotate()
    {
        _file_helper.close();

        std::string now_file_name = calc_filename(_base_filename, 0, _extension);
        std::string rotate_file_name = calc_filename(_base_filename, 1, _extension);
        if (details::file_helper::file_exists(rotate_file_name))
        {
            if (std::remove(rotate_file_name.c_str()) != 0)
            {
                throw spdlog_ex("rotating_file_sink: failed removing " + rotate_file_name);
            }
        }
        if (details::file_helper::file_exists(now_file_name) && std::rename(now_file_name.c_str(), rotate_file_name.c_str()))
        {
            throw spdlog_ex("rotating_file_sink: failed renaming " + now_file_name + " to " + rotate_file_name);
        }
        if(_max_files > 0)
        {
            std::vector< std::string > files;
            std::regex suffix_re("\\.[0-9]{8}\\.[0-9]{4}");

            std::string dirname;
            auto path_p = now_file_name.find_last_of("/\\");
            if(path_p == std::string::npos)
                dirname = ".";
            else
                dirname = now_file_name.substr(0, path_p + 1);
            DIR *dir = opendir(dirname.c_str());
            if(!dir)
            {
                throw spdlog_ex("rotating_file_sink: dir can't be open " + dirname);
            }
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL)
            {
                std::string file;
                if(path_p == std::string::npos)
                    file = ent->d_name;
                else
                    file = dirname + ent->d_name;
                if(file.substr(0, now_file_name.size()) != now_file_name)
                    continue;
                std::string suffix = file.substr(now_file_name.size());
                if (std::regex_match(suffix, suffix_re))
                {
                    files.push_back(file);
                }
            }
            closedir(dir);

            std::sort(files.begin(), files.end());
            for(int i = files.size() - _max_files; i >= 0 ; i--)
            {
                if (std::remove(files[i].c_str()) != 0)
                {
                    throw spdlog_ex("rotating_file_sink: failed removing " + files[i]);
                }
            }
        }

        _file_helper.reopen(true);
    }

    std::string _base_filename;
    std::string _extension;
    int _rotation_h;
    int _rotation_m;
    std::size_t _max_files;
    std::chrono::system_clock::time_point _rotation_tp;
    details::file_helper _file_helper;
};

typedef daily_file_sink<std::mutex> daily_file_sink_mt;
typedef daily_file_sink<details::null_mutex> daily_file_sink_st;
}
}