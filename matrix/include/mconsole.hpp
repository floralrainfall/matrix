#pragma once
#include <mhwabs.hpp>
#include <deque>

namespace mtx
{
    enum ConsoleLevel
    {
	CL_DEV,
	CL_INFO,
	CL_SOFTWARN,
	CL_WARN,
	CL_ERROR
    };
    
    struct ConsoleMessage
    {
	ConsoleLevel level;
	std::string message;
    };
	
    class Console
    {
	std::deque<ConsoleMessage> m_messageLog;
	ConsoleLevel m_level;
	FILE* m_output;
	FILE* m_errorOutput;
    public:
	Console();
	void pushMessage(ConsoleLevel level, std::string message);
	bool command(std::string command);
    };

    class ConsoleEventListener : public HWAPI::EventListener
    {
    public:
	
    };
};
