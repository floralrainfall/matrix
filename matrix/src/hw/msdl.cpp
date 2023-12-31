#include <hw/msdl.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

namespace mtx::sdl
{   
    SDLAPI::SDLAPI() : mtx::HWAPI()
    {
        DEV_MSG("initialized SDLAPI");

        m_firstWindow = 0;
	m_receiveEvents = true;

        if(SDL_Init(SDL_INIT_VIDEO) < 0)
            DEV_WARN("couldnt initialize SDL");
    }
    
    void SDLAPI::pumpOSEvents()
    {
	if(!m_receiveEvents)
	    return;
	
        SDL_Event e;
        SDLWindow* window = NULL;
        
        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_WINDOWEVENT:
                window = m_windows[e.window.windowID];                
                if(!window)
                    break;
                switch(e.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    for(auto listener : getListeners())
                        listener->onWindowClose(window->getEngineWindow());
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    for(auto listener : getListeners())
                        listener->onWindowSize(e.window.data1, e.window.data2, window->getEngineWindow());
                    break;
                }
                break;
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
            case SDL_MOUSEBUTTONDOWN:
                window = m_windows[e.button.windowID];
                if(!window)
                    break;
                for(auto listener : getListeners())
                    listener->onMouseDown(e.button.x, e.button.y, window->getEngineWindow());
                break;
            case SDL_MOUSEBUTTONUP:
                window = m_windows[e.button.windowID];
                if(!window)
                    break;
                for(auto listener : getListeners())
                    listener->onMouseUp(e.button.x, e.button.y, window->getEngineWindow());
                break;
            case SDL_MOUSEMOTION:
                window = m_windows[e.motion.windowID];
                if(!window)
                    break;
                for(auto listener : getListeners())
                {
                    listener->onMouseMove(e.motion.x, e.motion.y, window->getEngineWindow());
                    listener->onMouseMoveRel(e.motion.xrel, e.motion.yrel, window->getEngineWindow());
                }
                break;
            }
        }
    }

    void SDLAPI::showMessageBox(const char* title, const char* message, MessageBoxType type)
    {
        SDL_Window* window = NULL;
	if(m_firstWindow)
	    window = m_firstWindow->getSDLRef();
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

    SDLWindow::~SDLWindow()
    {
        if(m_sWind)
            SDL_DestroyWindow(m_sWind);
    }
    
    ConVar r_sdlvsync("r_sdlvsync", "", "1");
    
#ifdef GL_ENABLED
    void __r_sdlvsync_GL_Changed(ConVar* cvar)
    {
	bool vsync = cvar->getBool();
	if(vsync)
	{
	    int r = SDL_GL_SetSwapInterval(-1);
	    if(r == -1)
	    {
                DEV_SOFTWARN("OpenGL doesnt support adaptive vsync");
                SDL_GL_SetSwapInterval(1);
	    }
	    else 
		INFO_MSG("using adaptive vsync");
	}
	else
	    SDL_GL_SetSwapInterval(0);
    }
    
    SDL_GLContext SDLGLWindow::m_glCtxt = 0;
    SDLGLAPI::SDLGLAPI() : SDLAPI()
    {
	DEV_MSG("initialized SDLAPI... GL flavour");

	r_sdlvsync.setChangeFunction(__r_sdlvsync_GL_Changed);
    }

    void SDLGLAPI::shutdown()
    {
        if(SDLGLWindow::m_glCtxt)
        {
            SDL_GL_DeleteContext(SDLGLWindow::m_glCtxt);
            SDLGLWindow::m_glCtxt = 0;
        }
    }

    ConVar sdl_lessmessages("sdl_lessmessages");
    
    HWWindowReference* SDLGLAPI::newWindow(int resX, int resY, WindowType type)
    {
        SDLGLWindow* wref = new SDLGLWindow();
        wref->createWindow(glm::ivec2(resX, resY), (int)type);
        if(!m_firstWindow)
        {
            m_firstWindow = wref;
	    if(!sdl_lessmessages.getBool())
		showMessageBox("Matrix", "Matrix is licensed under the GNU GPLv3.\nA copy of the license should have been included with your binary.\nUsing SDLGLAPI by Ryelow <endoh@endoh.ca>");
        }
        m_windows[SDL_GetWindowID(wref->getSDLRef())] = wref;
        return (HWWindowReference*)wref;
    }
    
    void SDLGLWindow::createWindow(glm::ivec2 size, int type)
    {
        m_windowSize = size;

        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        int wflags = SDL_WINDOW_OPENGL;
        switch((HWAPI::WindowType)type)
        {
        case HWAPI::HWWT_FULLSCREEN:
            wflags |= SDL_WINDOW_FULLSCREEN;
            break;
        case HWAPI::HWWT_NORMAL_RESIZABLE:
            wflags |= SDL_WINDOW_RESIZABLE;
            break;
        default:
            break;
        }

        m_sWind = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, size.x, size.y, wflags);

        if(!m_glCtxt)
            m_glCtxt = SDL_GL_CreateContext(m_sWind);
        gladLoadGLLoader(SDL_GL_GetProcAddress);

        SDL_GL_MakeCurrent(m_sWind, m_glCtxt);

        bool usevsync = r_sdlvsync.getBool();

        if(usevsync)
        {
            int r = SDL_GL_SetSwapInterval(-1);
            if(r == -1)
            {
                DEV_SOFTWARN("OpenGL doesnt support adaptive vsync");
                SDL_GL_SetSwapInterval(1);
            }
            else 
                INFO_MSG("using adaptive vsync");
        }
        else
            SDL_GL_SetSwapInterval(0);
            
        INFO_MSG("OpenGL loaded");
        INFO_MSG("vendor: %s", glGetString(GL_VENDOR))
        INFO_MSG("renderer: %s", glGetString(GL_RENDERER));
        INFO_MSG("version: %s", glGetString(GL_VERSION));

        DEV_ASSERT(m_sWind);
        DEV_ASSERT(m_glCtxt);

        glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	
        glDepthFunc(GL_LESS);  
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

        glEnable(GL_CULL_FACE);
    }

    void SDLGLWindow::beginFrame()
    {
        SDL_GL_MakeCurrent(m_sWind, m_glCtxt);
        m_frameBegin = std::clock();
    }
    
    void SDLGLWindow::endFrame()
    {
        m_deltaTime = float(clock() - m_frameBegin) / CLOCKS_PER_SEC;
        SDL_GL_SwapWindow(m_sWind);
    }
#endif

#ifdef VK_ENABLED
#define HWAPI_VK (dynamic_cast<vk::VKAPI*>(App::getHWAPI()))
    
    void __r_sdlvsync_VK_Changed(ConVar* cvar)
    {
	DEV_SOFTWARN("__r_sdlvsync_VK_Changed is not implemented");
    }
    
    SDLVKAPI::SDLVKAPI() : SDLAPI()
    {
	DEV_MSG("initializing SDLVKAPI");
	r_sdlvsync.setChangeFunction(__r_sdlvsync_VK_Changed);
    }

    void SDLVKAPI::shutdown()
    {

    }

    HWWindowReference* SDLVKAPI::newWindow(int resX, int resY,
					   WindowType type)
    {
        SDLVKWindow* wref = new SDLVKWindow();
        wref->createWindow(glm::ivec2(resX, resY), (int)type);
        if(!m_firstWindow)
        {
            m_firstWindow = wref;
	    if(!sdl_lessmessages.getBool())
		showMessageBox("Matrix", "Matrix is licensed under the GNU GPLv3.\nA copy of the license should have been included with your binary.\nUsing SDLVKAPI by Ryelow <endoh@endoh.ca>");
        }
        m_windows[SDL_GetWindowID(wref->getSDLRef())] = wref;
        return (HWWindowReference*)wref;
    }
    
    void SDLVKWindow::createWindow(glm::ivec2 size, int type)
    {
        m_windowSize = size;

	m_sWind = SDL_CreateWindow("Matrix Vulkan",
				   0, 0, size.x, size.y,
				   SDL_WINDOW_SHOWN |
				   SDL_WINDOW_VULKAN);

	unsigned int extension_count = 0;
	const char** extension_names = 0;

	SDL_Vulkan_GetInstanceExtensions(m_sWind,
					 &extension_count,
					 NULL);
	extension_names = (const char**)malloc(sizeof(char*) * extension_count);
	SDL_Vulkan_GetInstanceExtensions(m_sWind,
					 &extension_count,
					 extension_names);

	HWAPI_VK->firstTimeInit(extension_names, extension_count);

	SDL_Vulkan_CreateSurface(m_sWind,
				 (SDL_vulkanInstance)HWAPI_VK->getInstance(),
				 (SDL_vulkanSurface*)&m_surface);
	m_presentQueue = HWAPI_VK->getSurfacePresentQueue(m_surface);
	m_graphicsQueue = HWAPI_VK->getGraphicsQueue();

	vkGetDeviceQueue(HWAPI_VK->getDevice(),
			 m_presentQueue, 0,
			 &m_vkPresentQueue);

	vk::VKSwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(HWAPI_VK->getPhysicalDevice(),
						  m_surface,
						  &details.capabilities);
	unsigned int format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(HWAPI_VK->getPhysicalDevice(),
					     m_surface,
					     &format_count,
					     NULL);
	if(format_count != 0)
	{
	    details.formats.resize(format_count);
	    vkGetPhysicalDeviceSurfaceFormatsKHR(HWAPI_VK->getPhysicalDevice(),
						 m_surface,
						 &format_count,
						 details.formats.data());
	}

	unsigned int present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(HWAPI_VK->getPhysicalDevice(),
						  m_surface,
						  &present_mode_count,
						  NULL);
	if(present_mode_count != 0)
	{
	    details.presentModes.resize(present_mode_count);
	    vkGetPhysicalDeviceSurfacePresentModesKHR(HWAPI_VK->getPhysicalDevice(),
						      m_surface,
						      &present_mode_count,
						      details.presentModes.data());
	}

	VkSurfaceFormatKHR surface_format = details.formats[0];
	for(auto format : details.formats)
	{
	    if(format.format == VK_FORMAT_B8G8R8A8_SRGB &&
	       format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
	    {
		surface_format = format;
		break;
	    }	    
	}

	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for(auto _present_mode : details.presentModes)
	{
	    if(_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
	    {
		present_mode = _present_mode;
		break;
	    }
	}

	VkExtent2D extent;
	if(details.capabilities.currentExtent.width !=
	   std::numeric_limits<unsigned int>::max())
	{
	    extent = details.capabilities.currentExtent;
	}
	else
	{
	    int width, height;
	    VkExtent2D true_extent = {
		static_cast<unsigned int>(size.x),
		static_cast<unsigned int>(size.y)
	    };
	    true_extent.width = std::clamp(true_extent.width,
					   details.capabilities.minImageExtent.width,
					   details.capabilities.maxImageExtent.width);
	    true_extent.height = std::clamp(true_extent.height,
					   details.capabilities.minImageExtent.height,
					   details.capabilities.maxImageExtent.height);
	    extent = true_extent;
	}
	
	unsigned int image_count = details.capabilities.minImageCount
	    + 1;

	if(image_count > 0 && image_count <
	details.capabilities.maxImageCount)
	{
	    image_count = details.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = image_count;
	createInfo.imageFormat = surface_format.format;
	createInfo.imageColorSpace = surface_format.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	unsigned int queue_indices[] = {
	    m_graphicsQueue, m_presentQueue
	};

	if(m_graphicsQueue != m_presentQueue)
	{
	    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	    createInfo.queueFamilyIndexCount = 2;
	    createInfo.pQueueFamilyIndices = queue_indices;
	}
	else
	{
	    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	    createInfo.queueFamilyIndexCount = 0;
	    createInfo.pQueueFamilyIndices = NULL;
	}

	createInfo.preTransform =
	    details.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = present_mode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if(vkCreateSwapchainKHR(HWAPI_VK->getDevice(),
				&createInfo,
				NULL,
				&m_swapChain) != VK_SUCCESS)
	{
	    DEV_WARN("could not create vulkan swap chain");
	}
	else
	    DEV_MSG("created swap chain");

	unsigned int swap_image_count;
	vkGetSwapchainImagesKHR(HWAPI_VK->getDevice(),
				m_swapChain,
				&swap_image_count,
				NULL);
	m_swapChainImages.resize(swap_image_count);
	m_swapChainImageViews.resize(swap_image_count);
	vkGetSwapchainImagesKHR(HWAPI_VK->getDevice(),
				m_swapChain,
				&swap_image_count,
				m_swapChainImages.data());

	for(int i = 0; i < m_swapChainImages.size(); i++)
	{
	    VkImageViewCreateInfo imCreateInfo{};
	    imCreateInfo.sType =
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	    imCreateInfo.image =
		m_swapChainImages[i];
	    imCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    imCreateInfo.format = surface_format.format;
	    imCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	    imCreateInfo.subresourceRange.baseMipLevel = 0;
	    imCreateInfo.subresourceRange.levelCount = 1;
	    imCreateInfo.subresourceRange.baseArrayLayer = 0;
	    imCreateInfo.subresourceRange.layerCount = 1;

	    if(vkCreateImageView(HWAPI_VK->getDevice(),
				 &imCreateInfo, NULL,
				 &m_swapChainImageViews[i]) !=
				 VK_SUCCESS)
	    {
		DEV_ERROR("could not create image view");
	    }
	}
	DEV_MSG("created %i swap chain views", m_swapChainImages.size());
    }

    void SDLVKWindow::beginFrame()
    {
	m_frameBegin = std::clock();
	HWAPI_VK->setSurface(&m_surface);
    }

    void SDLVKWindow::endFrame()
    {
	m_deltaTime = float(clock() - m_frameBegin) / CLOCKS_PER_SEC;
    }
#endif
    
    void SDLWindow::setGrab(bool grab)
    {
        SDL_bool _g = SDL_FALSE;
        if(grab)
            _g = SDL_TRUE;
        SDL_SetWindowGrab(m_sWind, _g);
        SDL_SetRelativeMouseMode(_g);
    }

    void SDLWindow::setWindowTitle(const char* title)
    {
        DEV_ASSERT(title);

        SDL_SetWindowTitle(m_sWind, title);
    }
}

#ifdef MODULE_COMPILE
static mtx::HLHWAPIEntry hwapi_entries[] = {
#ifdef GL_ENABLED
    {
	.ctor = mtx::HWAPIConstructorDefault<mtx::sdl::SDLGLAPI>,
	.name = "sdl::SDLGLAPI"
    },
#endif
#ifdef VK_ENABLED
    {
	.ctor = mtx::HWAPIConstructorDefault<mtx::sdl::SDLVKAPI>,
	.name = "sdl::SDLVKAPI"
    }
#endif
};

extern "C" {
    mtx::HLResponse* __MatrixMain(mtx::HLContext context)
    {
	mtx::HLResponse* rsp = (mtx::HLResponse*)malloc(sizeof(mtx::HLResponse));
	switch(context.request)
	{
	case mtx::HLR_LIST_HWAPIS:
	    rsp->status = mtx::HLS_SUCCESS;
	    rsp->data.listHwapis.entries = hwapi_entries;
	    rsp->data.listHwapis.entryCount = sizeof(hwapi_entries) / sizeof(mtx::HLHWAPIEntry);
	    break;
	default:
	    rsp->status = mtx::HLS_UNSUPPORTED_REQUEST;
	    break;
	}
	return rsp;
    }
}
#endif
