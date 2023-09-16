#pragma once
#include <mwweb.hpp>

namespace mtx::world
{
    class User
    {
        std::string m_username;

        void createFromResponse(WebServiceResponse rsp);
    public:
        User(int userid, WebService* ws);

        std::string getUsername() { return m_username; }
    };
}