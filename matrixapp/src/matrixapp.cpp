#include <mapp.hpp>
#include <mhwabs.hpp>
#include <mdev.hpp>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <hw/msdl.hpp>

mtx::ConVar player_devmode("player_devmode", "", "0");

namespace mtxplayer
{
    class ImGuiComponent : public mtx::SceneComponent
    {
    public:
	ImGuiComponent()
	{
	    
	}

	virtual void renderComponent()
	{
	    ImGui_ImplOpenGL3_NewFrame();
	    ImGui_ImplSDL2_NewFrame();

	    ImGui::NewFrame();
	    ImGui::ShowDemoWindow();

	    ImGui::Render();
	    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
    };
};

int main(int argc, char** argv)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    
    std::string gamepath = "";
    for(int i = 1; i < argc; i++) {
	std::string arg = argv[i];
	if(arg[0] == '-')
	{
	    std::string name = arg.substr(1);
	    std::string value = argv[++i];
	    if(name == "game")
		gamepath = value;
	}
    }

    if(gamepath == "")
    {
	DEV_ERROR("-gamepath not specified");
	return -1;
    }
    
    mtx::HWAPILib lib(gamepath.c_str());

    char** new_argv = (char**)malloc(sizeof(char*)*argc+2);
    int new_argc = argc+2;
    memcpy(new_argv, argv, sizeof(char*)*argc);
    new_argv[new_argc-2] = (char*)"+r_hwapi";
    new_argv[new_argc-1] = (char*)"sdl::SDLGLAPI";
    
    mtx::App* app = lib.getMainRoutine()(new_argc, new_argv);
    mtx::sdl::SDLGLAPI* sdlapi = dynamic_cast<mtx::sdl::SDLGLAPI*>(mtx::App::getHWAPI());
    
    if(!app->getAppRunning())
    {
	throw std::runtime_error("main routine error");
    }

    app->init();
    mtx::Window* window;
    mtx::sdl::SDLGLWindow* view;
    mtx::Viewport* vp;
    if(app->getWindows().size() == 0)
    {
	app->setAppHeadless(true);
	vp = new mtx::Viewport(640, 480);
	window = app->newWindow(vp);
	view = dynamic_cast<mtx::sdl::SDLGLWindow*>(window->getHwWindow());
    }
    else
    {
	app->initGfx();
	view = dynamic_cast<mtx::sdl::SDLGLWindow*>(sdlapi->getFirstWindow());
	vp = new mtx::Viewport(640, 480);
	window = view->getEngineWindow();
	window->addViewport(vp);
    }

    mtx::SceneManager* uimgr = new mtx::SceneManager(app);
    mtx::SceneNode* cam = new mtx::SceneNode();
    cam->setParent(uimgr->getRootNode());
    cam->addComponent(new mtxplayer::ImGuiComponent());
    vp->setCameraNode(cam);
    vp->setClearColor(glm::vec4(0));

    ImGui_ImplOpenGL3_Init();
    ImGui_ImplSDL2_InitForOpenGL(view->getSDLRef(),
				 mtx::sdl::SDLGLWindow::getGlContext());
	    
    app->getScheduler()->start();
    while(app->getAppRunning())
    {
	app->beginFrame();

	for(int i = 0; i < app->getWindows().size(); i++) {
	    mtx::Window* window = app->getWindows().at(i);
	    window->frame();

	    if(ImGui::GetIO().WantCaptureMouse ||
	       ImGui::GetIO().WantCaptureKeyboard)
	    {
		sdlapi->setReceiveEvents(false);

		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
		    switch(e.type)
		    {
		    case SDL_QUIT:
			app->setAppRunning(false);
			break;
		    }
		    ImGui_ImplSDL2_ProcessEvent(&e);
		}
	    }
	    else
		sdlapi->setReceiveEvents(true);
	    
	    if(window->getShouldClose())
	    {
		app->setAppRunning(false);
	    }
	}
	    
	app->endFrame();
    }
	
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
	
    app->cleanup();
}
