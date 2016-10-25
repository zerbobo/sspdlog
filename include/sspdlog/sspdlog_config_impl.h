#ifndef SSPDLOG_CONFIG_IMPL_H
#define SSPDLOG_CONFIG_IMPL_H

#include <fstream>
#include <iostream>

namespace sspdlog
{

inline void SspdlogConfig::UpdateConfig(const std::map< std::string, std::string > &conf)
{
    for (auto &p : conf) {
        if (p.first == LOGGER_NAMES_KEY && _config_map.find(p.first) != _config_map.end())
            _config_map[p.first] += std::string(",") + p.second;
        else
            _config_map[p.first] = p.second;
    }
}

inline void SspdlogConfig::SetConfig(const std::map< std::string, std::string > &conf)
{
    _config_map = conf;
    for (auto &p : CONFIG_MAP_DEFAULT) {
        if (_config_map.find(p.first) == _config_map.end())
            _config_map[p.first] = p.second;
    }
}

inline bool SspdlogConfig::ReadConfigFromSimpleFile(const std::string &filename)
{
    auto config_map = CONFIG_MAP_DEFAULT;
    std::ifstream fs(filename);
    while (fs.good())
    {
        std::string oneline(""), first(""), second("");
        std::getline(fs, oneline);
        if (oneline == "" || oneline[0] == '#')
            continue;
        std::string::size_type pos = oneline.find('=');
        if (pos == std::string::npos || pos == 1 || pos + 1 >= oneline.size())
            continue;

        first = oneline.substr(0, pos);
        std::string::size_type n1 = first.find_first_not_of(" \t");
        std::string::size_type n2 = first.find_last_not_of(" \t");
        if (n1 == std::string::npos)
            continue;
        else
            first = first.substr(n1, n2 - n1 + 1);

        second = oneline.substr(pos + 1);
        std::string::size_type n3 = second.find_first_not_of(" \t");
        std::string::size_type n4 = second.find_last_not_of(" \t");
        if (n3 == std::string::npos)
            continue;
        else{
            if (second[n3] == '\"' && second[n4] == '\"')
            {
                n3++; n4--;
            }
            if (n3 > n4)
                second = "";
            else
                second = second.substr(n3, n4 - n3 + 1);
        }

        config_map[first] = second;
    }
    if (!fs.eof())
        return false;
    _config_map = config_map;
    return true;
}

inline void SspdlogConfig::PrintConfig2Console()
{
    std::cout << "The config for sspdlog is:" << std::endl;
    for (auto &p : _config_map)
        std::cout << "  "<< p.first << ":" << p.second << std::endl;
}

}

#endif