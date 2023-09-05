#include <mapp.hpp>
#include <mdev.hpp>

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
    }

    int App::main()
    {
        init();

        if(m_windows.size() == 0)
            DEV_MSG("there were 0 windows after init was called, we wont start loop");

        struct timeval timecheck;
        gettimeofday(&timecheck, NULL);
        m_appStart = (double)timecheck.tv_sec + ((double)timecheck.tv_usec) / 1e+6;

        while(m_windows.size() != 0)
        {
            tick();
            int p = 0;
            for(auto window : m_windows)
            {
                window->pumpEvent();
                window->frame();

                if(window->m_shouldClose)
                {
                    m_windows.erase(m_windows.begin() + p);

                    delete window;
                    break;
                }
            }
        }
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

    void Window::addListener(EventListener* listener)
    {
        m_listeners.push_back(listener);
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

    void Window::pumpEvent()
    {
        m_hwWindow->pumpOSEvents(this);
    }

    void Window::init()
    {

    }

    void Window::addViewport(Viewport* viewport)
    {
        m_viewports.push_back(viewport);
    }
}