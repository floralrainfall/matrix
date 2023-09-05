#pragma once
#include <string>
#include <map>

namespace mtx
{
    class ConfigFile
    {
        std::map<std::string, std::string> m_values;
    public:
        ConfigFile(const char* file);

        std::string getValue(const char* property);
    };
}