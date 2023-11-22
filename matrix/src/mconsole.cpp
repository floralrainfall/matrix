#include <mconsole.hpp>
#include <mcvar.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

namespace mtx
{
    ConVar cl_consolelevel("cl_consolelevel",
			   "0 = dev, 1 = info, "
			   "2 = soft warns, "
			   "3 = warns, "
			   "4 = errors",
			   "1");
    
    Console::Console(App* app)
    {
	m_app = app;
	m_output = stdout;
	m_errorOutput = stderr;
	m_visible = false;
	m_gfxInit = false;
    }
    
    void Console::pushMessage(ConsoleLevel level, std::string message)
    {
	if(this)
	    m_level = (ConsoleLevel)cl_consolelevel.getInt();
	
	ConsoleMessage m;
	m.level = level;
	m.message = message;
	std::string levelstring;
	const char* color = "";
       
	FILE* output = this ? m_output : stdout;
	FILE* errorOutput = this ? m_errorOutput : stderr;
	switch(level)
	{
	case CL_DEV:
	    levelstring = "DEV";
	    color = "\033[1;30m";
	    break;
	case CL_INFO:
	    levelstring = "INFO";
	    color = "\033[0;37m";
	    break;
	case CL_SOFTWARN:
	    levelstring = "WARN";
	    color = "\033[1;33m";
	    break;
	case CL_WARN:
	    levelstring = "WARN";
	    output = errorOutput;
	    color = "\033[1;33m";
	    break;
	case CL_ERROR:
	    levelstring = "ERROR";
	    output = errorOutput;
	    color = "\033[1;31m";
	    break;
	default:
	    levelstring = "????";
	    break;
	}

	std::string taskinfo = "";
	if(this && m_app)
	{	    
	    Scheduler* sched = m_app->getScheduler();
	    if(sched)
	    {
		SchedulerTask* curr_task = sched->getCurrentTask();
		if(curr_task)
		    taskinfo = ":" + curr_task->name;
		else
		    taskinfo = ":main";
	    }
	}

	ConsoleLevel filter = this ? m_level : CL_DEV;
	if(output && level >= filter)
	    fprintf(output, "%s[%s%s] %s%s",
		    color,
		    levelstring.c_str(), taskinfo.c_str(), message.c_str(),
		    "\033[0m");

	if(this)
	{
	    m_messageDirtyLock.lock();
	    m_messageLog.push_front(m);	    
	    // while(m_messageLog.size() > 5000)
	    // m_messageLog.pop_back();
	    m_messageDirty = true;
	    switch(level)
	    {
	    case CL_DEV:
		m_numDevMessages++;
		break;
	    case CL_INFO:
		m_numInfoMessages++;
		break;
	    case CL_WARN:
	    case CL_SOFTWARN:
		m_numWarnMessages++;
		setVisible(true);
		break;
	    case CL_ERROR:
		m_numErrorMessages++;
		setVisible(true);
		break;
	    default:
		break;
	    }	

	    m_messageDirtyLock.unlock();
	}
    }

    void Console::initGfx(SceneManager* scene, Viewport* viewport)
    {
	m_consoleTexture = App::getHWAPI()->loadCachedTexture("textures/console_background.png");
	m_viewport = viewport;
	glm::vec4 vp = viewport->getViewport();
	
	SceneNode* node = new SceneNode();
	m_consoleImage = new GUIImageComponent(m_consoleTexture);
	//m_consoleImage->setColor(glm::vec4(0.5,0.5,0.5,1.0));
	m_consoleImage->setOffset(glm::vec3(
				      0,
				      vp.w-((MAX_CONSOLE_LINES+1)*16)-16,
				      0.5));
	m_consoleImage->setAbsoluteSize(true);
	m_consoleImage->setSize(glm::vec2(
				    vp.z,
				    ((MAX_CONSOLE_LINES+2)*16)));
	m_consoleText = new GUITextComponent();
	m_consoleText->setFont(
	    App::getHWAPI()->loadCachedTexture(
		"textures/font.png"));
	m_consoleText->setCharacterSize(glm::ivec2(
					    8,16
					    ));
	m_consoleText->setOutline(glm::vec4(0.01,0.01,0.01,1));
	m_consoleText2 = new GUITextComponent();
	m_consoleText2->setFont(
	    App::getHWAPI()->loadCachedTexture(
		"textures/font.png"));
	m_consoleText2->setCharacterSize(glm::ivec2(
					    8,16
					    ));
	m_consoleText2->setText("Entropy Interactive (c) 2023-2024");
	m_consoleText2->setColor(glm::vec4(1.0,0.682,0.259,1.0));
	m_consoleText2->setOutline(glm::vec4(0.5,0.282,0.059,1.0));

	int tl = m_consoleText2->getText().size();
	m_consoleText2->setOffset(glm::vec3(
				      vp.z-(tl*8),
				      vp.w-16,
				      0.7));
	
	node->addComponent(m_consoleImage);
	node->addComponent(m_consoleText);
	node->addComponent(m_consoleText2);
	node->setParent(scene->getRootNode());

	m_gfxInit = true;
	setVisible(false);
    }

    void Console::tickGfx()
    {
	if(!m_visible)
	    return;


	glm::vec4 vp = m_viewport->getViewport();

	int cpl = (int)ceilf(vp.w / 8.0);
	m_consoleText->setCharactersPerLine(
	    cpl);

	double anim_pctg = (m_consoleAnimNext -
			    m_app->getExecutionTime());
	double anim_top = vp.w;
	double consoleh = ((MAX_CONSOLE_LINES+2)*16)*2;
	if(anim_pctg > 0)
	    anim_top += (anim_pctg * consoleh);

	   
	int tl = m_consoleText2->getText().size();
	m_consoleText2->setOffset(glm::vec3(
				      vp.z-(tl*8),
				      anim_top-16,
				      0.7));
	m_consoleImage->setOffset(glm::vec3(
				      0,
				      anim_top-((MAX_CONSOLE_LINES+1)*16)-16,
				      0.5));
	m_consoleImage->setSize(glm::vec2(
				    vp.z,
				    ((MAX_CONSOLE_LINES+2)*16)));
	m_consoleText->setOffset(glm::vec3(
				     0,
				     anim_top-16,
				     0.6
				     ));

	if(m_messageDirty)
	{
	    m_messageDirtyLock.lock();
	    std::string console_text;

	    int lines_printed = 0;
	    for(ConsoleMessage m : m_messageLog)
	    {
		if(lines_printed > MAX_CONSOLE_LINES)
		    break;
		if(m.level < m_level)
		    continue;
		std::string tc = "";
		switch(m.level)
		{
		case CL_SOFTWARN:
		case CL_WARN:
		    tc = "\x27\xfe\xfe\x01";
		    break;
		case CL_ERROR:
		    tc = "\x27\xfe\x01\x01";
		    break;
		case CL_INFO:
		    tc = "\x27\xfe\xfe\xfe";
		    break;
		case CL_DEV:
		    tc = "\x27\x70\x70\x70";
		    break;
		}
		console_text = tc + m.message + "\x27\xff\xff\xff" + console_text;
		lines_printed++;
	    }

	    m_consoleText->setText(console_text);
	    m_messageDirty = false;
	    m_messageDirtyLock.unlock();
	}
	
    }

    void Console::setVisible(bool vis)
    {
	m_visible = vis;

	if(m_gfxInit)
	{
	    m_consoleText->setVisible(vis);
	    m_consoleText2->setVisible(vis);
	    m_consoleImage->setVisible(vis);
	    m_consoleAnimNext = m_app->getExecutionTime() + 0.5;
	}
    }

    ConsoleEventListener::ConsoleEventListener(Console* console)
    {
	m_console = console;
    }
}
