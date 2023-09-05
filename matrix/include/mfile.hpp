#pragma once
#include <string>
#include <vector>
#include <cstdio>

namespace mtx
{
    class FileSystem
    {
        std::vector<std::string> m_searchDirectories;
    public:
        FileSystem();

        void addDirectory(const char* directory);
        std::FILE* open(const char* file, const char* mode);
        std::FILE* open(const char* file); // opens for reading
        std::string fullPath(const char* file);

        static size_t getFileSize(std::FILE* file);
    };
}