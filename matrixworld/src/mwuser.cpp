#include <mwuser.hpp>
#include <picojson.h>
#include <mdev.hpp>

namespace mtx::world
{
    void User::createFromResponse(WebServiceResponse* rsp)
    {
        picojson::value v;
        std::string err = picojson::parse(v, rsp->response);
        if(!err.empty())
            DEV_WARN(err.c_str())
        else
        {
	    m_userdata = v.get("user");
            m_username = m_userdata.get("name").get<std::string>();
        }
    }

    User::User(int userid, WebService* ws)
    {
        WebServiceResponse rsp = ws->getSync(("user/" +
					      std::to_string(userid)).c_str());
	DEV_MSG("%s",rsp.response);

	m_webService = ws;
	m_banner = 0;
	m_badge = 0;
	m_userId = userid;
        createFromResponse(&rsp);
    }

    void User::loadWebContent()
    {
	if(!m_userdata.get("banner").is<picojson::null>())
	{
	    std::string bannerUrl =
		m_userdata.get("banner").get<std::string>();
	    m_banner = m_webService->getWebTexture(
			bannerUrl.c_str());
	}
	if(!m_userdata.get("badge").is<picojson::null>())
	{
	    std::string badgeUrl =
		m_userdata.get("badge").get<std::string>();
	    m_badge = m_webService->getWebTexture(
		badgeUrl.c_str());
	}
    }
}
