#pragma once
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>

namespace mtx
{
    class App;
    typedef std::function<void(App*)> SchedulerMethod;

    
    struct SchedulerTask
    {
	SchedulerMethod method;
	std::string name;
	std::thread thread;
	double deltaTime;

	SchedulerTask(const SchedulerTask&);
	SchedulerTask(std::string name, SchedulerMethod method);
    };
    
    class Scheduler
    {
	std::vector<SchedulerTask*> m_tasks;
	std::thread m_schedulerThread;
	
	void exec();
	
	bool m_paused;
	bool m_stopped;
	
	float m_deltaTime;
	App* m_app;
    public:
	Scheduler(App* app);

	void start();
	void stop();

	void setPaused(bool pause) { m_paused = pause; }
	double getDeltaTime() { return m_deltaTime; }

	// if NULL, then not in any sched tasks
	SchedulerTask* getCurrentTask();

	void newTask(std::string name, SchedulerMethod method);
    };
}
