#include <mconsole.hpp>
#include <mcvar.hpp>

namespace mtx
{
    ConVar cl_consolelevel("cl_consolelevel",
			   "0 = dev, 1 = info, "
			   "2 = soft warns, "
			   "3 = warns, "
			   "4 = errors",
			   "1");
    
    Console::Console()
    {
	m_output = stdout;
	m_errorOutput = stderr;
    }
    
    void Console::pushMessage(ConsoleLevel level, std::string message)
    {
	m_level = (ConsoleLevel)cl_consolelevel.getInt();
	if(level < m_level)
	    return;
	ConsoleMessage m;
	m.level = level;
	m.message = message;
	std::string levelstring;
	const char* color = "";

	FILE* output = m_output;
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
	    output = m_errorOutput;
	    color = "\033[1;33m";
	    break;
	case CL_ERROR:
	    levelstring = "ERROR";
	    output = m_errorOutput;
	    color = "\033[1;31m";
	    break;
	default:
	    levelstring = "????";
	    break;
	}

	if(output)
	    fprintf(output, "%s[%s] %s%s",
		    color,
		    levelstring.c_str(), message.c_str(),
		    "\033[0m");
	if(this)
	    m_messageLog.push_front(m);
    }
}
