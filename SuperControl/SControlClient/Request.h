#pragma once
#pragma warning(disable:4267)
#include "ClientSocket.h"
#include "ClientController.h"
#include "MQueue.h"
#include "MThread.h"
#include <mutex>
#include <atomic>
#define WM_RE_SEND_PACK		(WM_USER + 101)
#define WM_RE_SEND_PACK_ACK (WM_USER + 102)

class CRequest : public CMFuncBase
{
private:
	CMQueue<CPacket>	m_quePackets;
	CMThread			m_thread;
	HANDLE				m_event;
	std::mutex			m_mutex;
	std::atomic<bool>	m_stop;
public:

	int ThreadMain()
	{
		while (m_stop)
		{
			//m_mutex.lock();
			if (m_quePackets.size() <= 0)
			{
				WaitForSingleObject(m_event, INFINITE);
			}
			//TRACE(" ThreadMain -> size %d\r\n", GetTickCount64(), m_quePackets.size());
			ResetEvent(m_event);
			CPacket pack;
			m_quePackets.pop_front(pack);
			_SendPacket(pack);
			//m_mutex.unlock();
		}
		return -1;
	}

	void _SendPacket(CPacket& pack)
	{
		
		CClientSocket clientSock;
		clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
		clientSock.SetBufferSize(1024 * 1024 * 2);
		clientSock.Send(pack);
		clientSock.DealCommand();
		clientSock.CloseSocket();
	}

	CRequest()
	{
		m_thread.Work(CMWork(this,(MT_FUNC)&CRequest::ThreadMain));
		m_thread.Start();
		m_event = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_stop = true;
	}

	~CRequest()
	{
		m_stop = false;
	}

	void SendPacket(WORD nCmd, BYTE* data, DWORD len)
	{
		
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_mutex.lock();
		if (m_quePackets.size() > 10)
		{
			CPacket pack;
			m_quePackets.pop_front(pack);
		}
		m_quePackets.push_back(CPacket(nCmd,data,len));

		int a = m_quePackets.size();
		TRACE("SendPacket ->  %d  size  %d\r\n", GetTickCount64(), a);
		static int count = 0;
		TRACE("SendPacket ->  %d  count  %d\r\n", GetTickCount64(), ++count);
		m_mutex.unlock();
		SetEvent(m_event);
	}

	void SendPacket(CPacket& packet)
	{
		TRACE(" push back ->  %d  size  %d\r\n", GetTickCount64(), m_quePackets.size());
		m_quePackets.push_back(packet);
		SetEvent(m_event);
	}
	
};
