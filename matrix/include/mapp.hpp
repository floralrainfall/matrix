#pragma once
#include <mscene.hpp>
#include <mview.hpp>
#include <mhwabs.hpp>
#include <mfile.hpp>
#include <sys/time.h>

namespace mtx
{
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

        // newWindow calls this for you
        void init();

        void setTitle(const char* title) { m_hwWindow->setWindowTitle(title); }

        void addViewport(Viewport* viewport);

        void frame();
        void pumpEvent();
        void setShouldClose(bool c) { m_shouldClose = c; }

        class EventListener
        {
        public:
            virtual void onCloseWindow(Window* window) = 0;
            virtual void onKeyDown(int key) = 0;
            virtual void onKeyUp(int key) = 0;
        };

        std::vector<EventListener*> getListeners() { return m_listeners; }
        void addListener(EventListener* listener);
    private:
        std::vector<EventListener*> m_listeners;
    };
    
    class App
    {
        static HWAPI* m_hwApi;
        static FileSystem* m_fileSystem;

        std::vector<Window*> m_windows;
        bool m_appRunning;
        double m_appStart;
    public:
        App();

        virtual void init() {};
        virtual void tick() {};
        double getExecutionTime();

        Window* newWindow(Viewport* viewport = 0);
        void initParameters(int argc, char** argv);
        int main();

        static HWAPI* getHWAPI() { return m_hwApi; }
        static FileSystem* getFileSystem() { return m_fileSystem; }
    };
}