#pragma once
#include <mfile.hpp>
#include <mcfg.hpp>
#include <mhwabs.hpp>
#include <curl/curl.h>
#include <mapp.hpp>
#include <mnet.hpp>
#include <map>

namespace mtx::world
{
    class WebService;
    class User;
    class WebServiceListener;

    struct WebServiceResponse
    {
        WebService* ws;
        char* response;
        size_t size;
        long httpcode;

        void fillRespData(CURL* curl);
        ~WebServiceResponse();
    };

    struct WebServiceServerStatus
    {
	char name[64];
	char desc[128];
	char game[64];
	int player_count;
	int player_max;
	ENetAddress address;
    };

    struct WebServiceClientToken
    {
	std::string token;
	float renewAt;
	bool server;
	bool valid;
    };
    
    class WebService
    {
        friend class WebServiceListener;

        mtx::ConfigFile m_cfg;
        std::string m_httpbase;
        std::string m_motd;
	/// this auth key should NOT be transmitted and stay local to
        /// this WebService. please generate client tokens instead
        /// (using generateClientToken, and renew with renewClientToken)
        std::string m_authkey;
	float m_nextAuthRenew;
        User* m_currentUser;
        CURL* m_curl;
	App* m_app;
        SceneManager* m_sceneManager;
        WebServiceListener* m_eventListener;

        static size_t writeCallback(char* buffer, size_t size, size_t nitems, void* userdata);
        size_t writeCallback2(WebServiceResponse* r, char* buffer, size_t size, size_t nitems);
    public:
        static void initCurl();
       
        WebService(const char* cfg = "matrixworld.cfg");
        ~WebService();

        void addToApp(App* app);
	void doMyThang(); // call this every tick()

        std::string getMOTD() { return m_motd; }
        User* getCurrentUser() { return m_currentUser; }
        std::string getUrl(std::string path);

	WebServiceClientToken generateClientToken(bool server);
	void renewClientToken(WebServiceClientToken* token);

        WebServiceResponse getSync(const char* url);
        WebServiceResponse postSync(const char* url, std::map<std::string, std::string> multipart = std::map<std::string, std::string>());
        WebServiceResponse postSync(const char* url, char* postdata = "");

        HWTextureReference* getWebTexture(const char* url);

	// call this a minimum of 1 time every 30 seconds to keep
	// server on master list
	void postServerStatus(WebServiceServerStatus status,
			      WebServiceClientToken token);
    };
}
