#pragma once
#include <mfile.hpp>
#include <mcfg.hpp>
#include <mhwabs.hpp>
#include <curl/curl.h>
#include <mapp.hpp>
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

    class WebService
    {
        friend class WebServiceListener;

        mtx::ConfigFile m_cfg;
        std::string m_httpbase;
        std::string m_motd;
        std::string m_authkey;
        User* m_currentUser;
        CURL* m_curl;
        SceneManager* m_sceneManager;
        WebServiceListener* m_eventListener;

        static size_t writeCallback(char* buffer, size_t size, size_t nitems, void* userdata);
        size_t writeCallback2(WebServiceResponse* r, char* buffer, size_t size, size_t nitems);
    public:
        static void initCurl();
       
        WebService(const char* cfg = "matrixworld.cfg");
        ~WebService();

        void addToApp(App* app);

        std::string getMOTD() { return m_motd; }
        User* getCurrentUser() { return m_currentUser; }
        std::string getUrl(std::string path);

        WebServiceResponse getSync(const char* url);
        WebServiceResponse postSync(const char* url, std::map<std::string, std::string> multipart = std::map<std::string, std::string>());
        WebServiceResponse postSync(const char* url, char* postdata = "");

        HWTextureReference* getWebTexture(const char* url);
    };
}