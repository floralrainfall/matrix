#include <hw/mmotif.hpp>
#include <mcvar.hpp>
#include <mdev.hpp>

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <Xm/MessageB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>

#ifndef GL_ENABLED
#error Motif requires GL
#endif

namespace mtx::motif
{
    ConVar motif_debug("motif_debug", "", "1");

    MotifAPI::MotifAPI()
    {
	XtSetLanguageProc (NULL, NULL, NULL);

	DEV_MSG("MotifAPI");
	m_firstWindow = 0;
    }
    
    void MotifAPI::beginFrame()
    {

    }

    void MotifAPI::endFrame()
    {

    }

    void MotifAPI::shutdown()
    {

    }

    void MotifAPI::pumpOSEvents()
    {
	for(auto window : m_windows)
	{
	    XtAppContext app = window->getAppContext();
	    
	    XEvent event;
	    while(XtAppPeekEvent(app, &event))
	    {
		switch(event.type)
		{
		case KeyPress:
		    for(auto l : getListeners())
			l->onKeyDown(XLookupKeysym(&event.xkey, 0));
		    break;
		case KeyRelease:
		    for(auto l : getListeners())
			l->onKeyUp(XLookupKeysym(&event.xkey, 0));
		    break;
		case ResizeRequest:
		    for(auto l : getListeners())
			l->onWindowSize(event.xresizerequest.width,
					event.xresizerequest.height,
					window->getEngineWindow());
		    break;
		default:		    
		    break;
		}
		XtDispatchEvent(&event);
		XtAppNextEvent(app, &event);
	    }
	}
    }

    void MotifAPI::showMessageBox(const char* title,
				  const char* message,
				  MessageBoxType type)
    {
	if(!m_firstWindow)
	    return;
	
	Widget dialog = XmCreateMessageDialog(m_firstWindow->getWidget(),
					      "dialog",
					      NULL,
					      0);

	int dialog_type = 0;
	switch(type)
	{
	case HWMBT_INFORMATION:
	default:
	    dialog_type = XmDIALOG_INFORMATION;
	    break;
	case HWMBT_WARNING:
	    dialog_type = XmDIALOG_WARNING;
	    break;
	case HWMBT_ERROR:
	    dialog_type = XmDIALOG_ERROR;
	    break;
	}
	
	XmString text = XmStringCreateLocalized((char*)message);
	XmString title_text = XmStringCreateLocalized((char*)title);
	XtVaSetValues(dialog,
		      XmNdialogType, dialog_type,
		      XmNdialogTitle, title_text,
		      XmNmessageString, text,
		      NULL);

	XmStringFree(text);
	XmStringFree(title_text);
	XtManageChild(dialog);
	XtRealizeWidget(dialog);
    }

    HWWindowReference* MotifAPI::newWindow(int resX, int resY,
					   WindowType type)
    {
	MotifWindow* wref = new MotifWindow();
	wref->createWindow(glm::ivec2(resX, resY), (int)type);
	if(!m_firstWindow)
	{	    
	    m_firstWindow = wref;
	    showMessageBox("MotifAPI", "Using the Motif frontend\n"
			   "GL3API render backend\n"
			   "by Ryelow (c) 2023-2024\n");
	}
	m_windows.push_back(wref);
	return wref;
    }
    
    void MotifWindow::createWindow(glm::ivec2 size, int type)
    {
	const char* argv[] = {
	    "./matrix"
	};
	int argc = sizeof(argv)/sizeof(char*);

	int gl_attribs[] = {
	    GLX_RGBA,
	    GLX_DEPTH_SIZE, 24,
	    GLX_DOUBLEBUFFER,
	    None
	};

	m_topLevel = XtVaOpenApplication(&m_app,
					 "Matrix",
					 NULL,
					 0,
					 &argc,
					 (char**)argv,
					 NULL,
					 sessionShellWidgetClass,
					 NULL);
	
	m_display = XtDisplay(m_topLevel);

	if(!(m_visInfo = glXChooseVisual(m_display,
					 DefaultScreen(m_display),
					 gl_attribs)))
	{
	    DEV_ERROR("no suitable GL visual");
	}
	
	DEV_MSG("using GL visual %i", m_visInfo->visualid);

	auto root = DefaultRootWindow(m_display);
	Colormap cmap = XCreateColormap(m_display,
					root,
					m_visInfo->visual,
					AllocNone);

	((SessionShellWidget)m_topLevel)->shell.visual = m_visInfo->visual;
	m_topLevel->core.colormap = cmap;
	
	
	m_gameFrame = XmCreateMainWindow(m_topLevel,
					 "frame",
					 NULL,
					 0);

	Widget rowcol = XmCreateRowColumn(m_gameFrame, "rowcolumn",
					  NULL, 0);
	XtVaSetValues(rowcol,
		      XmNpacking, XmPACK_COLUMN,
		      XmNnumColumns, 4,
		      XmNnumRows, 4,
		      XmNorientation, XmVERTICAL,
		      NULL);
	
	if(motif_debug.getBool())
	{
	    DEV_MSG("adding motif debug widgets");

	    XmString file_text =  XmStringCreateLocalized("File");
	    Widget menubar = XmVaCreateSimpleMenuBar(
		m_gameFrame, "menubar", NULL, 0);
	    // TODO: add stuff
	    XtManageChild(menubar);
	}

	m_glxWidget = XtCreateWidget("glxwidget_0-2_0-2",
				     coreWidgetClass,
				     rowcol,
				     NULL,
				     0);

	Widget frame = XmCreateFrame(rowcol,
				     "frame_3_0",
				     NULL,
				     0);
	
	m_glxWidget->core.width = 640;
	m_glxWidget->core.height = 480;
	
	m_context = glXCreateContext(m_display, m_visInfo,
				     NULL, GL_TRUE);

	XtManageChild(frame);	
	XtManageChild(rowcol);
	XtManageChild(m_glxWidget);
	XtManageChild(m_gameFrame);

	DEV_MSG("realizing");
	XtRealizeWidget(m_topLevel);
	
	m_glxWindow = m_glxWidget->core.window;
	glXMakeCurrent(m_display, m_glxWidget->core.window, m_context);
	gladLoadGLLoader((GLADloadproc)glXGetProcAddress);
    }

    void MotifWindow::beginFrame()
    {
	glXMakeCurrent(m_display, m_glxWindow, m_context);
    }

    void MotifWindow::endFrame()
    {
	glXSwapBuffers(m_display, m_glxWindow);
    }
}

#ifdef MODULE_COMPILE
static mtx::HLHWAPIEntry hwapi_entries[] = {
    {
	.ctor = mtx::HWAPIConstructorDefault<mtx::motif::MotifAPI>,
	.name = "motif::MotifAPI"
    }
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
