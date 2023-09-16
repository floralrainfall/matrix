#include <hw/msdl.hpp>
#include <mapp.hpp>
#include <mdev.hpp>
#include <glad/glad.h>

namespace mtx::sdl
{
    SDLAPI::SDLAPI() : GL3API()
    {
        DEV_MSG("initialized SDLAPI");

        m_firstWindow = 0;

        if(SDL_Init(SDL_INIT_VIDEO) < 0)
            DEV_WARN("couldnt initialize SDL");

        SDL_GL_LoadLibrary(NULL);
    }

    void SDLAPI::shutdown()
    {
        if(SDLWindow::m_glCtxt)
        {
            SDL_GL_DeleteContext(SDLWindow::m_glCtxt);
            SDLWindow::m_glCtxt = 0;
        }
    }
    
    void SDLAPI::pumpOSEvents()
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                for(auto listener : getListeners())
                    listener->onQuit();
                break;
            case SDL_KEYDOWN:
                for(auto listener : getListeners())
                    listener->onKeyDown(e.key.keysym.sym);
                break;
            case SDL_KEYUP: 
                for(auto listener : getListeners())
                    listener->onKeyUp(e.key.keysym.sym);
                break;
            }
        }
    }

    void SDLAPI::showMessageBox(const char* title, const char* message, MessageBoxType type)
    {
        SDL_Window* window = 0;
        int flags = 0;
        switch(type)
        {
        default:
        case HWMBT_INFORMATION:
            flags = SDL_MESSAGEBOX_INFORMATION;
            break;
        case HWMBT_ERROR:
            flags = SDL_MESSAGEBOX_ERROR;
            break;
        case HWMBT_WARNING:
            flags = SDL_MESSAGEBOX_WARNING;
            break;
        }
        if(m_firstWindow)
            m_firstWindow->getSDLRef();
        SDL_ShowSimpleMessageBox(flags, title, message, window);
    }

    HWWindowReference* SDLAPI::newWindow(int resX, int resY)
    {
        SDLWindow* wref = new SDLWindow();
        wref->createWindow(glm::ivec2(resX, resY));
        if(!m_firstWindow)
        {
            m_firstWindow = wref;
            showMessageBox("Matrix", "Matrix is licensed under the GNU GPLv3.\nA copy of the license should have been included with your binary.\nUsing SDLAPI by Ryelow <endoh@endoh.ca>");
        }
        return (HWWindowReference*)wref;
    }

    SDL_GLContext SDLWindow::m_glCtxt = 0;

    SDLWindow::~SDLWindow()
    {
        if(m_sWind)
            SDL_DestroyWindow(m_sWind);
    }

    void SDLWindow::createWindow(glm::ivec2 size)
    {
        m_windowSize = size;

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        m_sWind = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, SDL_WINDOW_OPENGL);

        if(!m_glCtxt)
            m_glCtxt = SDL_GL_CreateContext(m_sWind);
        gladLoadGLLoader(SDL_GL_GetProcAddress);

        SDL_GL_SetSwapInterval(0);

        DEV_MSG("OpenGL loaded");
        DEV_MSG("vendor: %s", glGetString(GL_VENDOR))
        DEV_MSG("renderer: %s", glGetString(GL_RENDERER));
        DEV_MSG("version: %s", glGetString(GL_VERSION));

        DEV_ASSERT(m_sWind);
        DEV_ASSERT(m_glCtxt);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);  

        glEnable(GL_CULL_FACE);
    }

    void SDLWindow::setWindowTitle(const char* title)
    {
        DEV_ASSERT(title);

        SDL_SetWindowTitle(m_sWind, title);
    }

    void SDLWindow::beginFrame()
    {
        SDL_GL_MakeCurrent(m_sWind, m_glCtxt);
        m_frameBegin = std::clock();
    }
    
    void SDLWindow::endFrame()
    {
        m_deltaTime = float(clock() - m_frameBegin) / CLOCKS_PER_SEC;
        SDL_GL_SwapWindow(m_sWind);
    }
}