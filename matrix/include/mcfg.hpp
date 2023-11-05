#pragma once
#include <string>
#include <map>

namespace mtx
{
    class ConfigFile
    {
        std::map<std::string, std::string> m_values;
        bool m_found;
    public:
        ConfigFile(const char* file, bool cvar = false);

        bool getFound() { return m_found; }
        std::string getValue(const char* property);
	void setValue(const char* property, std::string text);
    };
}
