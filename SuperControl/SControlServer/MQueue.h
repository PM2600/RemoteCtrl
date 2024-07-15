#pragma once

#include <Windows.h>
#include <list>
template<typename T>
class CMQueue
{
private:
	enum
	{
		PUSH = 1001,
		POP,
		SIZE,
		CLEAR,
	};
	struct M
	{
		HANDLE hEvent;
		T t;
		int wParam;
		M(T _t){
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			t = _t;
			wParam = -1;
		}
		~M()
		{
			CloseHandle(hEvent);
		}
	};
private:
	std::list<T>	lstData;
	HANDLE			m_hIocp;
	HANDLE			m_hThread;
private:
	static void ThreadEntry(void* arg)
	{
		CMQueue* thiz = (CMQueue*)arg;
		thiz->ThreadMain();
		_endthread();
	}
	void ThreadMain()
	{
		DWORD transferred;
		ULONG_PTR completionKey;
		LPOVERLAPPED lpOverlapped;
		while (GetQueuedCompletionStatus(m_hIocp, &transferred, &completionKey, &lpOverlapped, INFINITE))
		{
			if ((transferred == -1) && (completionKey != -1) && (lpOverlapped == NULL))
			{
				M* pM = (M*)completionKey;
				SetEvent(pM->hEvent);
				break;
			}

			switch (transferred)
			{
			case PUSH:
			{
				lstData.push_back(*(T*)completionKey);
				delete (T*)completionKey;
				break;
			}
			case POP:
			{
				M* pM = (M*)completionKey;
				if (lstData.size() > 0)
				{
					pM->t = lstData.front();
					lstData.pop_front();
				}
				else
				{
					pM->wParam = 0;
				}
				SetEvent(pM->hEvent);
				break;
			}
			case SIZE:
			{
				M* pM = (M*)completionKey;
				pM->wParam = lstData.size();
				SetEvent(pM->hEvent);
				break;
			}
			case CLEAR:
			{
				M* pM = (M*)completionKey;
				while (lstData.size() > 0)
				{
					lstData.pop_front();
				}
				SetEvent(pM->hEvent);
				break;
			}
			}
		}
	}
public:
	CMQueue()
	{
		m_hThread = (HANDLE) _beginthread(ThreadEntry, 0, this);
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	}

	~CMQueue()
	{
		clear();
		T t;
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, -1, (ULONG_PTR)&m, NULL))
		{
			::AfxMessageBox(L"~CMQueue Error1");
			return;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			::AfxMessageBox(L"~CMQueue Error2");
			return;
		}
	}

	bool push_back(const T& t)
	{
		T* pT = new T(t);
		//memcpy(pT, &t, sizeof(T));
		if (!PostQueuedCompletionStatus(m_hIocp, PUSH, (ULONG_PTR)pT, NULL))
		{
			delete pT;
			return false;
		}
		return true;
	}

	bool pop_front(T& t)
	{
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, POP, (ULONG_PTR)&m, NULL))
		{
			return false;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return false;
		}
		if (m.wParam == 0)
		{
			return false;
		}
		t = m.t;
		return true;
	}

	size_t size()
	{
		T t;
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, SIZE, (ULONG_PTR)&m, NULL))
		{
			return 0;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return 0;
		}
		if (m.wParam != -1)
		{
			return m.wParam;
		}
		return 0;
	}

	bool clear()
	{
		T t;
		M m(t);
		if (!PostQueuedCompletionStatus(m_hIocp, CLEAR, (ULONG_PTR)&m, NULL))
		{
			return false;
		}
		if (WaitForSingleObject(m.hEvent, 100) != WAIT_OBJECT_0)
		{
			return false;
		}
		return true;
	}

};

