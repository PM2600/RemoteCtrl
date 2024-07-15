#pragma once

#include <pthread.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <unistd.h>

class CMFuncBase {};
typedef int (CMFuncBase::* MT_FUNC)();
typedef int (CMFuncBase::* MT_FUNC2)(void* arg);

class CMWork
{
public:
	CMFuncBase* thiz;
	MT_FUNC     func;
	MT_FUNC2    func2;
	void*		arg;
	CMWork(CMFuncBase* objThis, MT_FUNC objFunc) : func(nullptr), func2(nullptr)
	{
		thiz = objThis;
		func = objFunc;
	}
	CMWork(CMFuncBase* objThis, MT_FUNC2 objFunc, void* _arg = nullptr) : func(nullptr), func2(nullptr)
	{
		thiz = objThis;
		func2 = objFunc;
		arg = _arg;
	}
	int operator()()
	{
		if(func)
		{
			return (thiz->*func)();
		}
		if (func2)
		{
			return (thiz->*func2)(arg);
		}
		return -1;
	}
};

class CMThread
{
private:
	std::atomic<CMWork*> m_work;
	pthread_t            m_thread;
	bool				 m_run;
	static void* ThreadEntry(void* arg)
	{
		CMThread* thiz = (CMThread*)arg;
		thiz->ThreadMain();
		pthread_detach(pthread_self());
		pthread_exit(0);
	}
	void ThreadMain()
	{
		while (m_run)
		{
			while (m_work == NULL)
			{
				usleep(10000);
			}
			int ret = (*m_work)();
			if (ret != 0)
			{
				delete m_work;
				m_work = NULL;
			}
		}
	}
public:
	CMThread()
	{
		m_work = NULL;
		m_thread = -1;
		m_run = true;
	}
	~CMThread()
	{
		Stop();
	}
	
	void Work(const CMWork& work)
	{
		if (m_work != NULL)
		{
			delete m_work;
		}
		m_work = new CMWork(work);
	}
	bool Start()
	{
		pthread_create(&m_thread, NULL, ThreadEntry, this);
		if (m_thread == 0)
		{
			printf("线程启动失败\r\n");
			return false;
		}
		return true;
	}
	bool Stop()
	{
		if (m_run == false) 
		{
			return true;
		}
		m_run = false;
		void* thret;
		int ret = pthread_join(m_thread, &thret);
		if (ret == 0)
		{
			return true;
		}
		return false;
	}
	bool IsFree()
	{
		if (m_run && (m_work == NULL))
		{
			return true;
		}
		return false;
	}
};

class CMThreadPool
{
private:
	std::vector<CMThread*>	m_vecThreads;
	std::mutex				m_mutex;
public:
	CMThreadPool(int count)
	{
		m_mutex.lock();
		for (int i = 0; i < count; i++)
		{
			m_vecThreads.push_back(new CMThread());
		}
		m_mutex.unlock();
	}
	~CMThreadPool()
	{
		Stop();
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			delete m_vecThreads.at(i);
		}
		m_mutex.lock();
		m_vecThreads.clear();
		m_mutex.unlock();
	}
	bool Invoke()
	{
		bool isOk = true;
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (!m_vecThreads.at(i)->Start())
			{
				isOk = false;
				break;
			}
		}
		if (isOk)
		{
			return true;
		}
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			m_vecThreads.at(i)->Stop();
		}
		return false;
	}
	bool Stop()
	{
		bool isOk = true;
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (!m_vecThreads.at(i)->Stop())
			{
				isOk = false;
				break;
			}
		}
		if (isOk == false)
		{
			printf("线程关闭失败");
		}
		return isOk;
	}
	void DispatchWork(const CMWork& work)
	{
		for (size_t i = 0; i < m_vecThreads.size(); i++)
		{
			if (m_vecThreads.at(i)->IsFree())
			{
				m_vecThreads.at(i)->Work(work);
				break;
			}
		}
	}
};

