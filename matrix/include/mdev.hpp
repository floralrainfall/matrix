#pragma once

#include <cstdio>

namespace mtx
{
    static char tmp_dev_msg_buf[512] = {0};
}

#define DEV_MSG(x...)                                                       \
    { std::snprintf(mtx::tmp_dev_msg_buf, sizeof(mtx::tmp_dev_msg_buf), x); \
      std::printf("%s: %s\n", __PRETTY_FUNCTION__, mtx::tmp_dev_msg_buf); }

#define DEV_WARN(x...)                                                      \
    { std::snprintf(mtx::tmp_dev_msg_buf, sizeof(mtx::tmp_dev_msg_buf), x); \
      std::fprintf(stderr, "%s: %s\n", __PRETTY_FUNCTION__, mtx::tmp_dev_msg_buf); }

#define DEV_ASSERT(x)                           \
    if(!(x))                                    \
    {                                           \
        DEV_WARN("assert failed " #x " == 0");  \
    }