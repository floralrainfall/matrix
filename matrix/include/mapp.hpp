#pragma once
#include <mscene.hpp>
#include <mview.hpp>
#include <mhwabs.hpp>
#include <mfile.hpp>
#include <mnet.hpp>
#include <mcfg.hpp>
#include <sys/time.h>
#include <irrKlang.h>
#include <mschedule.hpp>
#include <mconsole.hpp>
#include <mcvar.hpp>

namespace mtx
{
    class RenderTarget
    {

    };
    
    class Window
    {
        friend class App;
        friend class HWWindowReference;

        std::vector<Viewport*> m_viewports;
        HWWindowReference* m_hwWindow;
        bool m_shouldClose;
    public:
        // use App::newWindow instead
        Window();
        ~Window();

        // newWindow calls this for you
        void init();

        void setTitle(const char* title) { m_hwWindow->setWindowTitle(title); }

        void addViewport(Viewport* viewport);
        std::vector<Viewport*> getViewports() { return m_viewports; }

        void frame();
        void setShouldClose(bool c) { m_shouldClose = c; }
        HWWindowReference* getHwWindow() { return m_hwWindow; }
    };
    
    class App
    {
    private:
        std::vector<NetInterface*> m_netInterfaces;        
    protected:
        static HWAPI* m_hwApi;
        static FileSystem* m_fileSystem;
	static irrklang::ISoundEngine* m_soundEngine;
        static double m_appStart;

        std::vector<Window*> m_windows;
        std::vector<SceneManager*> m_sceneManagers;
        bool m_appRunning;
        bool m_appHeadless;
	int m_appHeadlessFps;
        double m_appFrameStart;
        double m_deltaTime;
        double m_timeTillNextAnnouncement;
	double m_tickTime;
	double m_tickDeltaTime;
        std::string m_headlessStatus;
	ConfigFile* m_appConfig;
	Scheduler* m_scheduler;

	void thread_tick();
	void parseArguments(int argc, char** argv);
    public:
        App(int argc = 0, char** argv = NULL);

	static ConVarManager* conVarManager;
	static Console* console;
	
        virtual void init() {}; // allocate/initialize the game here
        virtual void initGfx() {}; // initialize graphics stuff
        virtual void tick() {}; // do stuff inbetween frames here
        virtual void tickGfx() {}; // do rendering stuff between frames here
        virtual void stop() {}; // deallocate all resources here

        static double getExecutionTime();
	
	/// please see App::getSchedulerTime before us1ing this method
	/// this is the time the main game loop takes to render a
	/// frame
	double getDeltaTime() { return m_deltaTime; }
	/// this is the time the scheduler takes to do every
	/// task. with only Tick as a task this should be approximate to
	/// getTickTime but can change. remember that the scheduler
	/// runs on a seperate thread, and using getDeltaTime will
	/// return the wrong delta for your probable purposes (unless
	/// you really need it)
	double getSchedulerTime()
	{
	    return m_scheduler->getDeltaTime();
	}
	/// this is the time the App::thread_tick method takes to
	/// run. this includes: sending/receiving network messages,
	/// the scene tick, and the app tick. this is why you should
	/// not use methods that interface with the video drawing
	/// HWAPI's as its not very garunteed that they will allow out
	/// of thread accesses
	double getTickTime() { return m_tickDeltaTime; }
	double& getTickRate() { return m_tickTime; }

	Scheduler* getScheduler() { return m_scheduler; }
	
	/**
	 * If this is set to false, the main loop will stop.
	 */
        void setAppRunning(bool s) { m_appRunning = s; }

	/**
	 * If this is set to false, the headless status messages will
	 * be printing.
	 */
        void setAppHeadless(bool s) { m_appHeadless = s; }

	/**
	 * Adds a scene manager to be automatically ticked by the App.
	 */
        void addSceneManager(SceneManager* m) { m_sceneManagers.push_back(m); }

	/**
	 * Creates a new window, visible to the user. After creating,
	 * the Viewport should have a Camera SceneNode, so it can
	 * render the SceneManager the node is within.
	 *
	 * @param viewport The viewport that the window will use. If
	 * there is none, you must initialize the Window's hardware
	 * window reference by yourself, and set it's 'Engine Window'
	 * via HWWindowReference::setEngineWindow for it to work.
	 */
        Window* newWindow(Viewport* viewport = 0, HWAPI::WindowType type = HWAPI::HWWT_NORMAL);

        NetServer* newServer(ENetAddress address);
        NetClient* newClient();

	/**
	 * Gets the Matrix configuration file
	 *
	 * This is basically <resource directory>/matrix.cfg, and is
	 * loaded at the start of execution
	 */
	ConfigFile* getConfig() { return m_appConfig; }

	/**
	 * Sets the status string of the headless print.
	 * This will appear right after the message before, with no
	 * spaces.
	 */
        void setHeadlessStatus(std::string s) { m_headlessStatus = s; };

	/**
	 * This runs the event loop for Matrix. You can pass the
	 * return code to the return of main().
	 */
        int main();

        static HWAPI* getHWAPI() { return m_hwApi; }
        static FileSystem* getFileSystem() { return m_fileSystem; }
	static irrklang::ISoundEngine* getSoundEngine()
	{
	    return m_soundEngine;
	};
    };
}
