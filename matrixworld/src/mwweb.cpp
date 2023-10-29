#include "mwweb.hpp"
#include <mdev.hpp>
#include <mwuser.hpp>
#include <cstring>
#include <picojson.h>

namespace mtx::world
{
    static bool global_init = false;

    class WebServiceListener : public HWAPI::EventListener
    {
        WebService* m_webService;
    public:
        WebServiceListener(WebService* s)
        {
            m_webService = s;
        }

        virtual void onQuit()
        {
            if(m_webService->m_currentUser)
                m_webService->postSync("auth/logout", {{"token", m_webService->m_authkey}});
        }

        virtual void onKeyDown(int k)
        {

        }

        virtual void onKeyUp(int k)
        {
            
        }
    };

    WebService::WebService(const char* cfg) : m_cfg(cfg)
    {
        initCurl();
        m_sceneManager = 0;
        m_eventListener = 0;
	m_currentUser = 0;

        if(!m_cfg.getFound())
            DEV_WARN("could not load WebService config file %s", cfg);
        
        m_httpbase = m_cfg.getValue("httpbase");
        DEV_MSG("using httpbase %s", m_httpbase.c_str());

        m_curl = curl_easy_init();
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeCallback);

        WebServiceResponse motdresp = getSync("motd");
        if(motdresp.response)
        {
            DEV_MSG("MOTD: %s", motdresp.response);
            m_motd = motdresp.response;

            ConfigFile logincfg = ConfigFile("WebService_Private.cfg");
            if(!logincfg.getFound())
                DEV_WARN("You are missing your login configuration file! Please create one in any of your resources folders and configure it accordingly");
            std::string token = logincfg.getValue("user_token");
            if(token != "null")
            {
                std::string email = logincfg.getValue("user_email");
                WebServiceResponse loginresp = postSync("auth/authorize", {
                    {"email", email},
                    {"token", token}
                });
                if(loginresp.httpcode != 200)               
                {
                    DEV_WARN("failed login '%s'", loginresp.response);
                }
                else
                {
                    picojson::value v;
                    std::string err = picojson::parse(v,
						      loginresp.response);

                    if(!err.empty())
			DEV_WARN(err.c_str())
                    else
                    {
                        DEV_MSG("logged in");
			picojson::value k = v.get("key");
			m_nextAuthRenew = v.get("renew_in").get<double>();
                        m_authkey = k.get("key").get<std::string>();
                        m_currentUser = new User((int)k.get("user_id").get<double>(), this);
                        DEV_MSG("username: %s", m_currentUser->getUsername().c_str());
                    }
                }
            }
        }
        else
            DEV_WARN("couldnt get MOTD from %s", m_httpbase.c_str());
    }
    
    WebService::~WebService()
    {
        curl_easy_cleanup(m_curl);
    }

    WebServiceResponse::~WebServiceResponse()
    {
        free(response);
    }

    void WebServiceResponse::fillRespData(CURL* curl)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpcode);
    }

    size_t WebService::writeCallback(char* buffer, size_t size, size_t nitems, void* userdata)
    {
        WebServiceResponse* wsr = (WebServiceResponse*)userdata;
        return wsr->ws->writeCallback2(wsr, buffer, size, nitems);
    }
    
    size_t WebService::writeCallback2(WebServiceResponse* r, char* buffer, size_t size, size_t nitems)
    {
        size_t realsize = size * nitems;
        char* ptr = (char*)realloc(r->response, r->size + realsize + 1);
        if(ptr == NULL)
        {
            DEV_WARN("we ran out of memory");
            return 0;
        }
        r->response = ptr;
        std::memcpy(&(r->response[r->size]), buffer, realsize);
        r->size += realsize;
        r->response[r->size] = 0;
        return realsize;
    }

    void WebService::initCurl()
    {
        if(!global_init)
        {
            global_init = true;
            curl_global_init(CURL_GLOBAL_ALL);
            DEV_MSG("initialized CURL");
        }
    }

    WebServiceResponse WebService::getSync(const char* url)
    {
        CURL* pc = curl_easy_duphandle(m_curl);
        WebServiceResponse r;
        r.response = 0;
        r.size = 0;
        r.ws = this;

        curl_easy_setopt(pc, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(pc, CURLOPT_URL, getUrl(url).c_str());
        curl_easy_setopt(pc, CURLOPT_WRITEDATA, &r);
        curl_easy_perform(pc);

        r.fillRespData(pc);

        curl_easy_cleanup(pc);        
        return r;
    }

    WebServiceResponse WebService::postSync(const char* url, std::map<std::string, std::string> multipart)
    {
        CURL* pc = curl_easy_duphandle(m_curl);
        WebServiceResponse r;
        r.response = 0;
        r.size = 0;
        r.ws = this;

        curl_mime* mp = curl_mime_init(pc);
        for(auto [k, v] : multipart)
        {
            curl_mimepart* part = curl_mime_addpart(mp);
            curl_mime_name(part, k.c_str());
            curl_mime_data(part, v.c_str(), CURL_ZERO_TERMINATED); 
        }

        curl_easy_setopt(pc, CURLOPT_MIMEPOST, mp);
        curl_easy_setopt(pc, CURLOPT_URL, getUrl(url).c_str());
        curl_easy_setopt(pc, CURLOPT_WRITEDATA, &r);
        curl_easy_perform(pc);

        r.fillRespData(pc);

        curl_mime_free(mp);
        curl_easy_cleanup(pc);        
        return r;
    }

    WebServiceResponse WebService::postSync(const char* url, char* postdata)
    {
        CURL* pc = curl_easy_duphandle(m_curl);
        WebServiceResponse r;
        r.response = 0;
        r.size = 0;
        r.ws = this;

        curl_easy_setopt(pc, CURLOPT_POSTFIELDS, postdata);
        curl_easy_setopt(pc, CURLOPT_URL, getUrl(url).c_str());
        curl_easy_setopt(pc, CURLOPT_WRITEDATA, &r);
        curl_easy_perform(pc);

        r.fillRespData(pc);

        curl_easy_cleanup(pc);        
        return r;
    }

    HWTextureReference* WebService::getWebTexture(const char* url)
    {
        HWTextureReference* rf = 0;
        rf = App::getHWAPI()->loadCachedTexture(url,false);
        if(!rf)
        {
            rf = App::getHWAPI()->newTexture();
            WebServiceResponse rsp = getSync(url);
            if(rsp.httpcode != 200)
            {
                DEV_WARN("unable to get web texture %s (%s)", url, rsp.response);
                return NULL;
            }
            rf->uploadCompressedTexture(rsp.size, rsp.response);
            App::getHWAPI()->addTextureToCache(rf, url);
        }
        return rf;
    }

    std::string WebService::getUrl(std::string path)
    {
        std::string url;
        std::string http = "https://";
        if(path.substr(0,http.size()) == http)
            url = path;
        else
            url = m_httpbase + "/" + path;
        return url;
    }

    void WebService::addToApp(App* app)
    {
	m_app = app;
        m_sceneManager = new SceneManager(app);
        m_eventListener = new WebServiceListener(this);
	m_nextAuthRenew = app->getExecutionTime() + m_nextAuthRenew;
        App::getHWAPI()->addListener(m_eventListener);
    }

    void WebService::postServerStatus(WebServiceServerStatus status,
				      WebServiceClientToken token)
    {
	if(!token.server)
	{
	    DEV_MSG("attempt to announce on a non-server token");
	    return;
	}
	    
	char hn[128];
	int r = enet_address_get_host(&status.address, hn, 128);
	if(r < 0)
	    enet_address_get_host_ip(&status.address, hn, 128);
	WebServiceResponse rsp = postSync("server/update", {
		{"name",status.name},
		{"desc",status.desc},
		{"game",status.game},
		{"player_count",std::to_string(status.player_count)},
		{"player_max",std::to_string(status.player_max)},
		{"server_ip",hn},
		{"server_port",std::to_string(status.address.port)},
		{"auth_key",m_authkey},
		{"token",token.token}
	    });
	if(rsp.httpcode != 200)
	{
	    DEV_MSG("couldnt announce server, r: %i, msg: %s",
		    rsp.httpcode, rsp.response);
	}
	else
	{
	    DEV_MSG("announced server (200 OK)");
	}
    }

    WebServiceClientToken WebService::generateClientToken(bool server)
    {
	WebServiceClientToken tk;
	tk.valid = false;
	WebServiceResponse rsp = postSync("auth/newtoken", {
		{"auth_key",m_authkey},
		{"server",std::to_string(server)}
	    });
	if(rsp.httpcode != 200)
	{
	    DEV_MSG("couldnt get client token, r: %i, msg: %s",
		    rsp.httpcode, rsp.response);
	}
	else
	{
	    picojson::value v;
	    std::string err = picojson::parse(v, rsp.response);
	    if(!err.empty())
		DEV_WARN(err.c_str())
	    else
	    {
		tk.token = v.get("key").get<std::string>();
		tk.server = server;
		tk.renewAt = v.get("renew_at").get<double>();
		tk.valid = true;
	    }
	}
	return tk;
    }

   

    void WebService::doMyThang()
    {
	if(m_app)
	{
	    if(m_app->getExecutionTime() > m_nextAuthRenew)
	    {
		// TODO: renew
	    }
	}
    }
}
