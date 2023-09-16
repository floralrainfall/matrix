#include <mcfg.hpp>
#include <mfile.hpp>
#include <mapp.hpp>
#include <cstring>
#include <mdev.hpp>

namespace mtx
{
    ConfigFile::ConfigFile(const char* file)
    {
        std::FILE* f = App::getFileSystem()->open(file);
        if(f)
        {
            char line[512];
            while(std::fgets(line, 512, f))
            {
                if(line[0] == '#')
                    continue;
                std::string name = std::strtok(line, "=");
                std::string text = std::strtok(NULL, "\n");
                m_values[name] = text;
            }
            m_found = true;
        }
        else
            m_found = false;
    }

    std::string ConfigFile::getValue(const char* property)
    {
        try{
            return m_values.at(std::string(property));
        }catch(std::exception e){
            return "null";
        }
    }
}