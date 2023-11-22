#pragma once
#include <mhwabs.hpp>
#include <mgui.hpp>
#include <mview.hpp>
#include <deque>
#include <mutex>

#define MAX_CONSOLE_LINES 9

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

    class App;
    class Console    
    {
	App* m_app;
	
	size_t m_numDevMessages;
	size_t m_numInfoMessages;
	size_t m_numWarnMessages;
	size_t m_numErrorMessages;

	double m_consoleAnimNext;
	
	std::deque<ConsoleMessage> m_messageLog;
	bool m_messageDirty;
	bool m_visible;
	bool m_gfxInit;
	std::mutex m_messageDirtyLock;

	GUITextComponent* m_consoleText;
	GUITextComponent* m_consoleText2;
	GUIImageComponent* m_consoleImage;
	HWTextureReference* m_consoleTexture;
	Viewport* m_viewport;
	
	ConsoleLevel m_level;
	FILE* m_output;
	FILE* m_errorOutput;
    public:
	Console(App* app);
	void pushMessage(ConsoleLevel level, std::string message);
	void tickGfx();
	void initGfx(SceneManager* scene, Viewport* viewport);
	bool command(std::string command);

	size_t getNumDevMessages() { return m_numDevMessages; }
	size_t getNumInfoMessages() { return m_numInfoMessages; }
	size_t getNumWarnMessages() { return m_numWarnMessages; }
	size_t getNumErrorMessages() { return m_numErrorMessages; }

	void setVisible(bool vis);
    };

    class ConsoleEventListener : public HWAPI::EventListener
    {
	Console* m_console;
    public:
	ConsoleEventListener(Console* console);
    };
};
