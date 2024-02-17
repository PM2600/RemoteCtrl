#pragma once

template<class T>
class CQueue
{
	//线程安全的队列（利用IOCP实现）
public:
	CQueue() {};
	~CQueue() {};
	bool PushBack(const T& data);
	bool PopFront(T& data);
	size_t Size();
	void Clear();
private:
	static void threadEntry(void* arg);
	void threadMain();
private:
	std::list<T> m_lstData;
	HANDLE m_hCompletionPort;
	HANDLE m_hThread;

public:
	typedef struct IocpParam {
		int nOperator;
		T strData;
		_beginthread_proc_type cbFunc;
		HANDLE hEvent;
		IocpParam(int op, const char* pData, _beginthread_proc_type cb = NULL) {
			nOperator = op;
			strData = pData;
			cbFunc = cb;
		}
		IocpParam() {
			nOperator = -1;
		}
	}PPARAM;

	enum {
		EQEmpty,
		EQPush,
		EQPop,
		EQSize
	};

};