#include <mapp.hpp>
#include <mdev.hpp>
#include <algorithm>
#include <chrono>
#include <thread>
#include <functional>
#include <SDL2/SDL_ttf.h>

// put your frontends here

#ifdef SDL_ENABLED
#include <hw/msdl.hpp>
#endif

#ifdef MOTIF_ENABLED
#include <hw/mmotif.hpp>
#endif

#define HWAPI_ENTRY_DEFAULT(x) \
    api_map2[ #x ] = HWAPIConstructorDefault<x>;

namespace mtx
{
    HWAPI* App::m_hwApi = 0;
    FileSystem* App::m_fileSystem = 0;
    irrklang::ISoundEngine* App::m_soundEngine = 0;
    double App::m_appStart = 0.0;
    Console* App::console = 0;

    ConVar r_hwapi = ConVar("r_hwapi",
			    "",
			    "");
    ConVar tickrate = ConVar("tickrate",
			     "",
			     "");
    ConVar snd_disable = ConVar("snd_disable",
				"",
				"");
    ConVar hwapimodule = ConVar("hwapimodule",
				"",
				"");

    // dynamic hwapi table
    static std::map<std::string, HWAPIConstructor> api_map;
    
    // put your frontends here too
    
    static std::map<std::string, HWAPIConstructor>  __getSupportedApis()
    {
	std::map<std::string, HWAPIConstructor> api_map2(api_map);
#ifdef SDL_ENABLED
#ifdef GL_ENABLED
	HWAPI_ENTRY_DEFAULT(sdl::SDLGLAPI);
#endif
#ifdef VK_ENABLED
	HWAPI_ENTRY_DEFAULT(sdl::SDLVKAPI);
#endif
#endif
#ifdef MOTIF_ENABLED
	HWAPI_ENTRY_DEFAULT(motif::MotifAPI);
#endif
	return api_map2;
    }

    std::string App::getCompileString()
    {
	std::string relinfo;
#ifdef NDEBUG
	relinfo = "RELEASE";
#else
	relinfo = "DEBUG";
#endif
	
	return "Matrix Alpha " + relinfo;
    }
    
    std::string App::getExtraCompileInfoString()
    {
	return "";
    }

    App::App(int argc, char** argv)
    {
	m_scheduler = 0;
	console = new Console(this);

	INFO_MSG("%s %s",
		 getCompileString().c_str(),
		 getExtraCompileInfoString().c_str());
	
        if(m_fileSystem == 0)
            m_fileSystem = new FileSystem();
	m_appConfig = new ConfigFile("matrix.cfg", true);
	parseArguments(argc, argv);

	if(hwapimodule.getBool())
	{
	    HWAPILib lib(hwapimodule.getString().c_str());
	    auto libctrs = lib.getConstructors();
	    api_map.insert(libctrs.begin(), libctrs.end());
	}
	
	if(__getSupportedApis().size() == 0)
	{
	    DEV_ERROR("there are no hwapis");
	    exit(-1);
	}
	
	std::string rmode = r_hwapi.getString();
	if(rmode == "")
	{
	    rmode = __getSupportedApis().begin()->first;
	    r_hwapi.setString(rmode);
	}
	    

#ifndef NDEBUG
#ifndef CONSOLE_DISABLE_WARNING
	DEV_SOFTWARN("Matrix uses Console::Console. If Console::initGfx "
		     "and Console::tickGfx aren't called, then it may not "
		     "work correctly. Define CONSOLE_DISABLE_WARNING in "
		     "the matrix project to disable this warning");
#endif
#endif
	
        m_appRunning = true;

	if(m_hwApi == 0)
        {
	    rmode = r_hwapi.getString();
	    DEV_MSG("using hwapi %s", rmode.c_str());

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
		DEV_ERROR("no hwapi found");
		std::string supported_hwapi_string;
		
		for(auto api : __getSupportedApis())
		    supported_hwapi_string += api.first + " ";

		INFO_MSG("supported hwapis: %s", supported_hwapi_string.c_str());
		m_appRunning = false;
	    }
	    else
	    {
		m_hwApi->addListener(new ConsoleEventListener(console));
	    }

            struct timeval timecheck;
            gettimeofday(&timecheck, NULL);
            m_appStart = (double)timecheck.tv_sec +
		((double)timecheck.tv_usec) / 1e+6;
        }
	std::string sound_en = snd_disable.getString();
	if(m_soundEngine == 0 && sound_en != "true")
	{
	    m_soundEngine = irrklang::createIrrKlangDevice();
	}
        m_appHeadless = false;
	m_appHeadlessFps = 3000;
	
	m_tickTime = 1.0/tickrate.getFloat();

	m_scheduler = new Scheduler(this);
	m_scheduler->newTask("Tick", &App::thread_tick);

	if(TTF_Init() < 0) {
	    DEV_SOFTWARN("couldn't initialize ttf");
	}
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

    void App::parseArguments(int argc, char** argv)
    {
	for(int i = 1; i < argc; i++) {
	    std::string arg = argv[i];
	    // set ConVar... suspiciously like Source!
	    if(arg[0] == '+')
	    {
		std::string name = arg.substr(1);
		std::string value = argv[++i];
		conVarManager->conVarCommand(name, value);
	    }
	    else if(arg[0] == '-') // options
	    {
		std::string name = arg.substr(1);
		std::string value = argv[++i];
		// TODO: this
	    }
	    else
		DEV_SOFTWARN("bad command %s", arg.c_str());
	}
	conVarManager->listConVars();
    }

    void App::thread_tick()
    {
	double tick_start = getExecutionTime();
	std::chrono::time_point<std::chrono::system_clock> start =
	    std::chrono::system_clock::now();
	for(auto i : m_netInterfaces)
	    i->eventFrame();
	for(auto m : m_sceneManagers)
	    m->tickScene();
	tick();
	m_hwApi->pumpOSEvents();

	double exec_time = getExecutionTime() - tick_start;
	double sleep_time = m_tickTime - exec_time;
	//DEV_MSG("%f %f",exec_time, sleep_time);
	if(sleep_time > 0.0)
	    std::this_thread::sleep_for(std::chrono::duration<double>(
					    sleep_time
					    ));
	
	m_tickDeltaTime = getExecutionTime() - tick_start;
    }

    int App::main()
    {
	try
	{
	    if(!m_appRunning)
	    {
		DEV_ERROR("some kind of error on App init, cleaning up");
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
	    
	    m_scheduler->start();
	    while(m_appRunning)
	    {
		beginFrame();

		for(int i = 0; i < m_windows.size(); i++) {
		    Window* window = m_windows.at(i);
		    window->frame();

		    if(window->m_shouldClose)
		    {
			m_windows.erase(m_windows.begin() + i);
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
		
		endFrame();
	    }
	}
	catch (std::exception& ex)
	{
	    DEV_ERROR("%s", ex.what());
	}
	catch(...)
	{
	    DEV_ERROR("unknown error");
	}

    cleanup:
        DEV_MSG("cleaning up");
	cleanup();

        INFO_MSG("goodbye, %i warnings and %i errors",
		 console->getNumWarnMessages(),
		 console->getNumErrorMessages());

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

    void App::beginFrame()
    {
	m_appFrameStart = getExecutionTime();

	if(m_windows.size() != 0)
	    tickGfx();

	App::getHWAPI()->frameStart();
    }

    void App::endFrame()
    {
	int p = 0;

	if(m_appHeadless)
	{
	    if(m_timeTillNextAnnouncement < m_appFrameStart)
	    {
		INFO_MSG("Time: %f, Tick DT: %f, Ticks/Second: %f%s",
			 getExecutionTime(),
			 getSchedulerTime(),
			 1.0 / getSchedulerTime(),
			 m_headlessStatus.c_str());
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

    void App::cleanup()
    {
	if(m_scheduler)
	    m_scheduler->stop();
	
	try {	    
	    stop();
	}
	catch(std::exception& ex)
	{
	    DEV_ERROR("%s", ex.what());
	}
	catch(...)
	{
	    DEV_ERROR("unknown error");
	}

        for(int i = 0; i < m_windows.size(); i++)
        {
            Window* window = m_windows.at(i);
            DEV_MSG("shutting down window %i", i);
            delete window;
        }        
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

            if(viewport->getCameraNode() &&
	       viewport->getCameraNode()->getScene())
            {
                viewport->updateView();

                SceneNode* cameraNode = viewport->getCameraNode();
                HWRenderParameter rp;
                rp.name = "projection";
                rp.data.m4 = viewport->getPerspective();
                rp.type = HWT_MATRIX4;                
                App::getHWAPI()->pushParam(rp);
		
                rp.name = "projection_inverse";
                rp.data.m4 = viewport->getPerspectiveInverse();
                rp.type = HWT_MATRIX4;                
                App::getHWAPI()->pushParam(rp);

                rp.name = "view";
                rp.data.m4 = viewport->getView();
                rp.type = HWT_MATRIX4;
                App::getHWAPI()->pushParam(rp);
		
                rp.name = "view_inverse";
                rp.data.m4 = viewport->getViewInverse();
                rp.type = HWT_MATRIX4;
                App::getHWAPI()->pushParam(rp);

                viewport->getCameraNode()
		    ->getScene()
		    ->renderScene(viewport->getCameraNode());
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
