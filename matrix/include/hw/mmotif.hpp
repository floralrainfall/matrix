#pragma once
#include <Xm/PushB.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <X11/Xlib.h>
#include <mhwabs.hpp>
#include <hw/mgl.hpp>
#include <GL/glx.h>

namespace mtx::motif
{    
    class MotifWindow : public mtx::HWWindowReference
    {
    protected:
	Display* m_display;
	XtAppContext m_app;
	XVisualInfo* m_visInfo;
	GLXContext m_context;
	Widget m_topLevel;
	Widget m_gameFrame;
	Widget m_glxWidget;
	Pixmap m_pixmap;
	GLXPixmap m_glxPixmap;
	long int m_glxWindow;
    public:
	XtAppContext getAppContext() { return m_app; };
	Widget getWidget() { return m_topLevel; };

	virtual void createWindow(glm::ivec2 size, int type);
	virtual void beginFrame();
	virtual void endFrame();
    };
    
    class MotifAPI : public gl::GL3API
    {
	std::vector<MotifWindow*> m_windows;
	MotifWindow* m_firstWindow;
    public:
	MotifAPI();
	virtual void pumpOSEvents();
	virtual void showMessageBox(const char* title,
				    const char* message,
				    MessageBoxType type = HWMBT_INFORMATION);
	virtual HWWindowReference* newWindow(int resX,
					     int resY,
					     WindowType type);
	virtual void beginFrame();
	virtual void endFrame();

	virtual void shutdown();
    };
}
