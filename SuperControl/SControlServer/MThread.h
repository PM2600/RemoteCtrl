#pragma once

#include "pch.h"
#include "framework.h"
#include <atomic>
#include <vector>
#include <mutex>

class CMFuncBase{};
typedef int (CMFuncBase::* MT_FUNC)();

class CMWork
{
public:
	CMFuncBase* thiz;
	MT_FUNC     func;
	CMWork(CMFuncBase* objThis, MT_FUNC objFunc)
	{
		thiz	= objThis;
		func	= objFunc;
	}
	int operator()()
	{
		return (thiz->*func)();
	}
};

class CMThread
{
private:
	std::atomic<CMWork*> m_work;
	HANDLE               m_thread;
	HANDLE				 m_event;
	bool				 m_run;
	static void ThreadEntry(void* arg)
	{
		CMThread* thiz = (CMThread*)arg;
		thiz->ThreadMain();
		_endthread();
	}
	void ThreadMain()
	{
		while (m_run)
		{
			WaitForSingleObject(m_event, INFINITE);

			int ret = (*m_work)();
			if (ret != 0)
			{
				delete m_work;
				m_work = NULL;
			}
			else
			{
				Notify(true);
			}
		}
	}
public:
	CMThread()
	{
		m_work		= NULL;
		m_thread	= INVALID_HANDLE_VALUE;
		m_event = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_run		= true;
	}
	~CMThread()
	{
		Stop();
	}
	void Notify(bool b)
	{
		if (b)
		{
			SetEvent(m_event);
		}
		else
		{
			ResetEvent(m_event);
		}
	}
	void Work(const CMWork& work)
	{
		if (m_work != NULL)
		{
			delete m_work;
		}
		m_work = new CMWork(work);
		Notify(true);
	}
	bool Start()
	{
		m_thread = (HANDLE)_beginthread(&CMThread::ThreadEntry,0,this);
		if (m_thread == NULL)
		{
			TRACE("线程启动失败\r\n");
			return false;
		}
		return true;
	}
	bool Stop()
	{
		if (m_run == false) return true;
		m_run = false;
		int ret = WaitForSingleObject(m_thread, 100);
		printf("stop ret : %d\r\n", ret);
		if (ret == WAIT_OBJECT_0)
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
		//assert(isOk);
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

