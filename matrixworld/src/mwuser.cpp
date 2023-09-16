#include <mwuser.hpp>
#include <picojson.h>
#include <mdev.hpp>

namespace mtx::world
{
    void User::createFromResponse(WebServiceResponse rsp)
    {
        picojson::value v;
        std::string err = picojson::parse(v, rsp.response);
        if(!err.empty())
            DEV_WARN(err.c_str())
        else
        {
            m_username = v.get("username").get<std::string>();
        }
    }

    User::User(int userid, WebService* ws)
    {
        WebServiceResponse rsp = ws->getSync(("user/" + std::to_string(userid)).c_str());
        createFromResponse(rsp);
    }
}