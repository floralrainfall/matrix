#include <mapp.hpp>
#include <mdev.hpp>
#include <algorithm>
#include <hw/msdl.hpp>

namespace mtx
{
    HWAPI* App::m_hwApi = 0;
    FileSystem* App::m_fileSystem = 0;

    App::App()
    {
        if(m_hwApi == 0)
            m_hwApi = new sdl::SDLAPI();
        if(m_fileSystem == 0)
            m_fileSystem = new FileSystem();
        m_appRunning = true;
        m_appHeadless = false;
    }

    int App::main()
    {
        init();

        struct timeval timecheck;
        gettimeofday(&timecheck, NULL);
        m_appStart = (double)timecheck.tv_sec + ((double)timecheck.tv_usec) / 1e+6;
        
        if(m_windows.size() == 0)
        {                        
            m_appHeadless = true;
            m_timeTillNextAnnouncement = 0.f;
        }

        while(m_appRunning)
        {
            for(auto m : m_sceneManagers)
                m->tickScene();
            tick();
            int p = 0;

            m_hwApi->pumpOSEvents();

            m_appFrameStart = getExecutionTime();
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
                    DEV_MSG("Time: %f, Tick DT: %f, Ticks/Second: %f", getExecutionTime(), m_deltaTime, 1.0 / m_deltaTime);
                    m_timeTillNextAnnouncement = m_appFrameStart + 5.f;
                }
            }

            m_deltaTime = getExecutionTime() - m_appFrameStart;
        }

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

    void App::initParameters(int argc, char** argv)
    {

    }

    double App::getExecutionTime()
    {
        struct timeval timecheck;
        gettimeofday(&timecheck, NULL);
        double time = (double)timecheck.tv_sec + ((double)timecheck.tv_usec) / 1e+6;
        return (time - m_appStart);
    }

    Window* App::newWindow(Viewport* vp)
    {
        Window* nWnd = new Window();
        if(vp)
        {
            nWnd->addViewport(vp);
            glm::vec4 vp_d = vp->getViewport();
            nWnd->m_hwWindow = getHWAPI()->newWindow((int)vp_d.z,(int)vp_d.w);           
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

                viewport->getCameraNode()->getScene()->renderScene();
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