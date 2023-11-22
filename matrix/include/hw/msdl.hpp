#pragma once

#ifndef SDL_ENABLED
#error "hw/msdl.hpp in use, but SDL_ENABLED not defined"
#endif

#ifdef GL_ENABLED
#include <hw/mgl.hpp>
#endif
#ifdef VK_ENABLED
#include <hw/mvk.hpp>
#include <SDL2/SDL_vulkan.h>
#endif
#include <SDL2/SDL.h>

namespace mtx::sdl
{
    class SDLWindow : public mtx::HWWindowReference
    {
    protected:
        SDL_Window* m_sWind;
    public:
        virtual ~SDLWindow();
        virtual void setGrab(bool grab);
        virtual void setWindowTitle(const char* title);
        SDL_Window* getSDLRef() { return m_sWind; }
    };

    class SDLAPI : public virtual HWAPI
    {
    protected:
        SDLWindow* m_firstWindow;
        std::map<unsigned int, SDLWindow*> m_windows;
	bool m_receiveEvents;
    public:
        SDLAPI();
        virtual void pumpOSEvents();
	virtual void showMessageBox(const char* title, const char*
    message, MessageBoxType type = HWMBT_INFORMATION);
	SDLWindow* getFirstWindow() { return m_firstWindow; }
	void setReceiveEvents(bool b) { m_receiveEvents = b; }
    };

#ifdef GL_ENABLED
    class SDLGLWindow : public SDLWindow
    {
        friend class SDLGLAPI;
        static SDL_GLContext m_glCtxt;
    public:
        virtual void createWindow(glm::ivec2 size, int type);
        virtual void beginFrame();
        virtual void endFrame();

	static SDL_GLContext getGlContext() { return m_glCtxt; }
    };
    
    class SDLGLAPI : public SDLAPI, public mtx::gl::GL3API
    {
    public:
        SDLGLAPI();
        virtual void shutdown();
        
        virtual HWWindowReference* newWindow(int resX, int resY, WindowType type);
    };
#endif

#ifdef VK_ENABLED
    class SDLVKWindow : public SDLWindow
    {
	friend class SDLVKAPI;
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapChain;
	int m_presentQueue;
	int m_graphicsQueue;
	VkQueue m_vkPresentQueue;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
    public:
        virtual void createWindow(glm::ivec2 size, int type);
        virtual void beginFrame();
        virtual void endFrame();
    };
    
    class SDLVKAPI : public SDLAPI, public mtx::vk::VKAPI
    {
    public:
	SDLVKAPI();
	virtual void shutdown();
        
        virtual HWWindowReference* newWindow(int resX, int resY, WindowType type);
    };
#endif
}
