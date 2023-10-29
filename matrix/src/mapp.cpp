#include <mapp.hpp>
#include <mdev.hpp>
#include <algorithm>
#include <chrono>
#include <thread>
#include <functional>

#ifdef SDL_ENABLED
#include <hw/msdl.hpp>
#endif

#define HWAPI_ENTRY_DEFAULT(x) \
    api_map[ #x ] = HWAPIConstructorDefault<x>;

namespace mtx
{
    HWAPI* App::m_hwApi = 0;
    FileSystem* App::m_fileSystem = 0;
    irrklang::ISoundEngine* App::m_soundEngine = 0;
    double App::m_appStart = 0.0;

    static std::map<std::string, HWAPIConstructor>  __getSupportedApis()
    {
	std::map<std::string, HWAPIConstructor> api_map;
#ifdef SDL_ENABLED
#ifdef GL_ENABLED
	HWAPI_ENTRY_DEFAULT(sdl::SDLGLAPI);
#endif
#ifdef VK_ENABLED
	HWAPI_ENTRY_DEFAULT(sdl::SDLVKAPI);
#endif
#endif
	return api_map;
    }

    App::App(int argc, char** argv)
    {
        if(m_fileSystem == 0)
            m_fileSystem = new FileSystem();
	m_appConfig = new ConfigFile("matrix.cfg");
        m_appRunning = true;
	for(int i = 1; i < argc; i += 2)
	{
	    if(i+1 >= argc)
	    {
		DEV_MSG("missing argument on last setting");
		break;
	    }
	    
	    m_appConfig->setValue(argv[i],argv[i+1]);
	}
	if(m_hwApi == 0)
        {
	    std::string rmode = m_appConfig->getValue("hwapi");
	    m_hwApi = 0;
	    for(auto api : __getSupportedApis())
	    {
		if(rmode == api.first)
		{
		    m_hwApi = api.second();
		    break;
		}
	    }

	    if(!m_hwApi)
	    {
		DEV_MSG("no hwapi found");
		std::string supported_hwapi_string;
		
		for(auto api : __getSupportedApis())
		    supported_hwapi_string += api.first + " ";

		DEV_MSG("supported hwapis: %s", supported_hwapi_string.c_str());
		m_appRunning = false;
	    }

            struct timeval timecheck;
            gettimeofday(&timecheck, NULL);
            m_appStart = (double)timecheck.tv_sec +
		((double)timecheck.tv_usec) / 1e+6;
        }
	if(m_soundEngine == 0)
	{
	    m_soundEngine = irrklang::createIrrKlangDevice();
	}
        m_appHeadless = false;
	m_appHeadlessFps = 3000;
    }

    NetServer* App::newServer(ENetAddress address)
    {
        NetServer* s = new NetServer(address);
        m_netInterfaces.push_back(s);
        return s;
    }

    NetClient* App::newClient()
    {
        NetClient* c = new NetClient();
        m_netInterfaces.push_back(c);
        return c;
    }

    int App::main()
    {
	if(!m_appRunning)
	{
	    DEV_MSG("some kind of error on App init, cleaning up");
	    goto cleanup;
	}
	
        init();
        
        if(m_windows.size() == 0)
        {                        
            m_appHeadless = true;
            m_timeTillNextAnnouncement = 0.f;
        }
        else
            initGfx();

        while(m_appRunning)
        {
            m_appFrameStart = getExecutionTime();
            for(auto i : m_netInterfaces)
                i->eventFrame();
            for(auto m : m_sceneManagers)
                m->tickScene();
            tick();
            if(m_windows.size() != 0)
                tickGfx();
            int p = 0;

            m_hwApi->pumpOSEvents();

            for(int i = 0; i < m_windows.size(); i++) {
                Window* window = m_windows.at(i);
                window->frame();

                if(window->m_shouldClose)
                {
                    m_windows.erase(m_windows.begin() + p);
                    if(m_windows.size() == 0 && m_appRunning)
                    {
                        DEV_MSG("last window closed, entering headless mode");
                        m_appHeadless = true;
                        m_timeTillNextAnnouncement = 0.f;
                    }

                    delete window;
                    break;
                }
            }

            if(m_appHeadless)
            {
                if(m_timeTillNextAnnouncement < m_appFrameStart)
                {
                    DEV_MSG("Time: %f, Tick DT: %f, Ticks/Second: %f%s", getExecutionTime(), m_deltaTime, 1.0 / m_deltaTime, m_headlessStatus.c_str());
                    m_timeTillNextAnnouncement = m_appFrameStart + 5.f;
                }
		if(m_appHeadlessFps != 0)
		{
		    double exec_time = getExecutionTime() - m_appFrameStart;
		    double sleep_time = 1.0 / (double)m_appHeadlessFps - exec_time;

		    std::this_thread::sleep_for(
			std::chrono::duration<double>(
			    sleep_time));
		}
            }
	    
            m_deltaTime = getExecutionTime() - m_appFrameStart;
        }

    cleanup:
        DEV_MSG("cleaning up");

        stop();

        // clean up time
        for(int i = 0; i < m_windows.size(); i++)
        {
            Window* window = m_windows.at(i);
            DEV_MSG("shutting down window %i", i);
            delete window;
        }        

        DEV_MSG("goodbye");

        return 0;
    }

    double App::getExecutionTime()
    {
        struct timeval timecheck;
        gettimeofday(&timecheck, NULL);
        double time = (double)timecheck.tv_sec + ((double)timecheck.tv_usec) / 1e+6;
        return (time - m_appStart);
    }

    Window* App::newWindow(Viewport* vp, HWAPI::WindowType type)
    {
        Window* nWnd = new Window();
        if(vp)
        {
            nWnd->addViewport(vp);
            glm::vec4 vp_d = vp->getViewport();
            nWnd->m_hwWindow = getHWAPI()->newWindow((int)vp_d.z,(int)vp_d.w,type);           
            nWnd->m_hwWindow->setEngineWindow(nWnd);
        }
        m_windows.push_back(nWnd);
        return nWnd;
    }

    Window::Window()
    {
        m_shouldClose = false;
    }

    Window::~Window()
    {
        delete m_hwWindow;
        for(auto vp : m_viewports)
            delete vp;
    }

    void Window::frame()
    {
        m_hwWindow->beginFrame();
        for(auto viewport : m_viewports)
        {
            viewport->beginViewportFrame();
            
            DEV_ASSERT(viewport->getCameraNode());

            if(viewport->getCameraNode() && viewport->getCameraNode()->getScene())
            {
                viewport->updateView();

                SceneNode* cameraNode = viewport->getCameraNode();
                HWRenderParameter rp;
                rp.name = "projection";
                rp.data.m4 = viewport->getPerspective();
                rp.type = HWT_MATRIX4;                
                App::getHWAPI()->pushParam(rp);

                rp.name = "view";
                rp.data.m4 = viewport->getView();
                rp.type = HWT_MATRIX4;
                App::getHWAPI()->pushParam(rp);

                viewport->getCameraNode()->getScene()->renderScene(viewport->getCameraNode());
            }

            App::getHWAPI()->clearParams();
        }
        m_hwWindow->endFrame();
    }

    void Window::init()
    {

    }

    void Window::addViewport(Viewport* viewport)
    {
        m_viewports.push_back(viewport);
    }
}
