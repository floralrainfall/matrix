#pragma once

#include <cstdio>
#include <mhwabs.hpp>
#include <mapp.hpp>

namespace mtx
{
    static char tmp_dev_msg_buf[512] = {0};
    static char tmp_dev_msg_buf2[512] = {0};
}

#define DEV_MSG_FORMAT "%s:%i: %s\n", __BASE_FILE__, __LINE__

#define DEV_MSG(x...)                                                       \
    { std::snprintf(mtx::tmp_dev_msg_buf, sizeof(mtx::tmp_dev_msg_buf), x); \
      std::printf(DEV_MSG_FORMAT, mtx::tmp_dev_msg_buf); }

#define DEV_WARN(x...)                                                               \
    { std::snprintf(mtx::tmp_dev_msg_buf, sizeof(mtx::tmp_dev_msg_buf), x);          \
      std::snprintf(mtx::tmp_dev_msg_buf2, sizeof(mtx::tmp_dev_msg_buf2), DEV_MSG_FORMAT, mtx::tmp_dev_msg_buf);   \
      std::fprintf(stderr, "%s", mtx::tmp_dev_msg_buf2);                                              \
      App::getHWAPI()->showMessageBox("Matrix Warning", mtx::tmp_dev_msg_buf2, mtx::HWAPI::HWMBT_WARNING); } 

#define DEV_ASSERT(x)                           \
    if(!(x))                                    \
    {                                           \
        DEV_WARN("assert failed " #x " == 0");  \
    }

#define DEV_ASSERT2(x, o, y)                                            \
    if(!(x o y))                                                        \
    {                                                                   \
        DEV_WARN("assert failed " #x " (%i) " #o " " #y " (%i)", x, y); \
    }