#pragma once
#include <mwweb.hpp>
#include <mhwabs.hpp>
#include <picojson.h>

namespace mtx::world
{
    class User
    {
	WebService* m_webService;
        std::string m_username;
	int m_userId;
	HWTextureReference* m_banner;
	HWTextureReference* m_badge;
	picojson::value m_userdata;

        void createFromResponse(WebServiceResponse* rsp);
    public:
        User(int userid, WebService* ws);
	void loadWebContent();

        std::string getUsername() { return m_username; }
	HWTextureReference* getBanner() { return m_banner; }
	HWTextureReference* getBadge() { return m_badge; }
    };
}
