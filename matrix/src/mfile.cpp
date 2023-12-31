#include <mfile.hpp>
#include <mdev.hpp>
#include <unistd.h>

namespace mtx
{
    FileSystem::FileSystem()
    {
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
        DEV_MSG("cwd: %s",cwd);
        addDirectory("resources");
        addDirectory("matrix");
        addDirectory("../resources");
        addDirectory("~/.local/share/matrix");
        addDirectory("/usr/share/matrix");
    }

    void FileSystem::addDirectory(const char* directory)
    {
        DEV_MSG("adding new directory %s", directory);
        m_searchDirectories.push_back(std::string(directory));
    }

    std::string FileSystem::fullPath(const char* file)
    {
	if(file[0] == '/')
	    return file;
	
        std::FILE* f = 0;
        for(auto dir : m_searchDirectories)
        {
            f = fopen((dir + "/" + file).c_str(), "r");
            if(f)
            {
                fclose(f);
                return (dir + "/" + file);
            }
        }
        DEV_SOFTWARN("could not find file %s", file);
        return file;
    }

    std::FILE* FileSystem::open(const char* file, const char* mode)
    {
        std::FILE* f = 0;
        for(auto dir : m_searchDirectories)
        {
            f = fopen((dir + "/" + file).c_str(), mode);
            if(f)
                return f;
        }
        DEV_SOFTWARN("could not open file %s mode %s", file, mode);
        return NULL;
    }

    std::FILE* FileSystem::open(const char* file)
    {
        return open(file, "r");
    }

    size_t FileSystem::getFileSize(std::FILE* file)
    {
        size_t p = ftell(file);
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, p, SEEK_SET); // reset it so it seems seamless to the caller
        return size;
    }
}
