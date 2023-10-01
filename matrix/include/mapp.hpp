#pragma once
#include <mscene.hpp>
#include <mview.hpp>
#include <mhwabs.hpp>
#include <mfile.hpp>
#include <mnet.hpp>
#include <sys/time.h>

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
        static double m_appStart;

        std::vector<Window*> m_windows;
        std::vector<SceneManager*> m_sceneManagers;
        bool m_appRunning;
        bool m_appHeadless;
        double m_appFrameStart;
        double m_deltaTime;
        double m_timeTillNextAnnouncement;
    public:
        App();

        virtual void init() {}; // allocate/initialize the game here
        virtual void initGfx() {}; // initialize graphics stuff
        virtual void tick() {}; // do stuff inbetween frames here
        virtual void stop() {}; // deallocate all resources here

        static double getExecutionTime();
        double getDeltaTime() { return m_deltaTime; }

        void setAppRunning(bool s) { m_appRunning = s; }
        void setAppHeadless(bool s) { m_appHeadless = s; }
        void addSceneManager(SceneManager* m) { m_sceneManagers.push_back(m); }

        Window* newWindow(Viewport* viewport = 0, HWAPI::WindowType type = HWAPI::HWWT_NORMAL);

        NetServer* newServer(ENetAddress address);
        NetClient* newClient(ENetAddress address);

        void initParameters(int argc, char** argv);
        int main();

        static HWAPI* getHWAPI() { return m_hwApi; }
        static FileSystem* getFileSystem() { return m_fileSystem; }
    };
}