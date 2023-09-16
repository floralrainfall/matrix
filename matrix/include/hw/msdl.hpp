#pragma once

#include <hw/mgl.hpp> 
#include <SDL2/SDL.h>

namespace mtx::sdl
{
    class SDLWindow : public mtx::HWWindowReference
    {
        friend class SDLAPI;
        SDL_Window* m_sWind;
        static SDL_GLContext m_glCtxt;
    public:
        virtual ~SDLWindow();

        SDL_Window* getSDLRef() { return m_sWind; }

        virtual void createWindow(glm::ivec2 size);
        virtual void setWindowTitle(const char* title);
        virtual void beginFrame();
        virtual void endFrame();
    };

    class SDLAPI : public mtx::gl::GL3API
    {
        SDLWindow* m_firstWindow;
    public:
        SDLAPI();
        virtual void shutdown();
        virtual void showMessageBox(const char* title, const char* message, MessageBoxType type = HWMBT_INFORMATION);
        virtual void pumpOSEvents();
        
        virtual HWWindowReference* newWindow(int resX, int resY);
    };
}