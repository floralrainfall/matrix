#include <mschedule.hpp>
#include <mapp.hpp>
#include <mdev.hpp>

namespace mtx
{
    SchedulerTask::SchedulerTask(std::string name,
				 SchedulerMethod method)
    {
	this->name = name;
	this->method = method;
    }
    
    Scheduler::Scheduler(App* app)
    {
	m_app = app;
    }

    void Scheduler::start()
    {
	m_paused = false;
	m_stopped = false;
	
	m_schedulerThread = std::thread(&Scheduler::exec, this);
    }

    void Scheduler::exec()
    {
	while(!m_stopped)
	{
	    float start_time = m_app->getExecutionTime();
	    for(SchedulerTask* task : m_tasks)
	    {
		task->thread = std::thread(task->method, m_app);
	    }
	    
	    for(SchedulerTask* task : m_tasks)
	    {
		if(task->thread.joinable())
		    task->thread.join();
	    }
	    m_deltaTime = m_app->getExecutionTime() - start_time;
	}
    }

    void Scheduler::stop()
    {
	m_stopped = true;
	if(m_schedulerThread.joinable())
	    m_schedulerThread.join();
	
	for(SchedulerTask* task : m_tasks)
	{
	    DEV_MSG("stopping thread %s...", task->name.c_str());
	    if(task->thread.joinable())
		task->thread.join();
	    delete task;
	}
	m_tasks.clear();
    }

    void Scheduler::newTask(std::string name, SchedulerMethod method)
    {
	m_tasks.push_back(new SchedulerTask(name, method));
    }
};
