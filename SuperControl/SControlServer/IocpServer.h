#pragma once
#include "MThread.h"
#include "CmdProcessor.h"
#include "MQueue.h"
#include "Screenshot.h"
#include <map>
#include <list>
#include <MSWSock.h>
#pragma warning(disable:4244)
#pragma warning(disable:4477)

class CMClient;
class CIocpServer;
extern CCmdProcessor cmdProc;
extern CMQueue<CPacket> lstScreenPcks;
extern std::mutex mtx_;

void Screen2(CPacket& pack);

enum CMOperator
{
	MAccept = 10001,
	MRecv,
	MSend,
	MClose
};

class CMOverlapped 
{	
public:
	OVERLAPPED					m_overlapped;
	CMClient*					m_client;
	int							m_operator;
	DWORD						m_received;
	std::vector<char>			m_buffer;
	WSABUF						m_wsaBuf;
	CMOverlapped()
	{
	}
};

template<CMOperator op>
class AcceptOverlapped : public CMFuncBase , public CMOverlapped
{
public:
	AcceptOverlapped()
	{
		m_operator = op;
		m_buffer.resize(1024 * 256);
	}

	int Func();
	
};
typedef AcceptOverlapped<MAccept> ACCEPTOVERLAPPED;

template<CMOperator op>
class RecvOverlapped : public CMFuncBase, public CMOverlapped
{
public:
	RecvOverlapped()
	{
		m_operator = op;
		m_buffer.resize(1024 * 256);
	}

	int Func();
	
};
typedef RecvOverlapped<MRecv> RECVOVERLAPPED;

template<CMOperator op>
class SendtOverlapped : public CMFuncBase , public CMOverlapped
{
public:
	std::list<CPacket> lstSendPacks;
	SendtOverlapped()
	{
		m_operator = op;
		m_buffer.resize(1024 * 256);
	}

	int Func();
};
typedef SendtOverlapped<MSend> SENDOVERLAPPED;

template<CMOperator op>
class CloseOverlapped : public CMOverlapped, public CMFuncBase
{
public:
	CloseOverlapped()
	{
		m_operator = op;
		m_buffer.resize(1024 * 256);
	}

	int Func()
	{
		return -1;
	}
};
typedef CloseOverlapped<MClose> CLOSEOVERLAPPED;

class CMClient
{
public:
	std::shared_ptr<ACCEPTOVERLAPPED>	m_accept;
	std::shared_ptr<RECVOVERLAPPED>		m_recv;
	std::shared_ptr<SENDOVERLAPPED>		m_send;
	std::shared_ptr<CLOSEOVERLAPPED>	m_close;
	SOCKET								m_servSocket;
	SOCKET								m_clntSocket;
	CIocpServer*					    m_iocpServer;
	sockaddr_in							m_laddr;
	sockaddr_in							m_raddr;
	HANDLE								m_iocp;
	std::mutex							m_mutex;
	ULONGLONG							m_tick;
	CMClient(CIocpServer* serv,HANDLE iocp);
};

class CIocpServer : public CMFuncBase
{
public:
	CMThreadPool				m_pool;
	HANDLE						m_HIOCP;
	std::mutex					m_mutex;
	SOCKET						m_servSocket;
	std::map<ULONGLONG, CMClient*> m_mapClients;
private:
	int IocpMain()
	{
		DWORD transferred;
		ULONG_PTR completionKey;
		LPOVERLAPPED lpOverlapped;
		while (GetQueuedCompletionStatus(m_HIOCP, &transferred, &completionKey, &lpOverlapped, INFINITE))
		{
			ULONGLONG t1{}, t2{};
			//结束IOCP
			if ((transferred == -1) && (completionKey == -1) && (lpOverlapped == NULL))
			{
				break;
			}
			//根据信息，做对应的处理
			CMOverlapped* pOverlapped = (CMOverlapped*) lpOverlapped;
			switch (pOverlapped->m_operator)
			{
				//让线程池处理连接事物
				case CMOperator::MAccept:
				{
					ACCEPTOVERLAPPED* po = (ACCEPTOVERLAPPED*)pOverlapped;
					m_pool.DispatchWork(CMWork(po, (MT_FUNC)&ACCEPTOVERLAPPED::Func));
					break;
				}
				//让线程池处理接收事物
				case CMOperator::MRecv:
				{
					RECVOVERLAPPED* po = (RECVOVERLAPPED*)pOverlapped;
					m_pool.DispatchWork(CMWork(po, (MT_FUNC)&RECVOVERLAPPED::Func));
					break;
				}
				//让线程池处理发送事物
				case CMOperator::MSend:
				{
					SENDOVERLAPPED* po = (SENDOVERLAPPED*)pOverlapped;
					m_pool.DispatchWork(CMWork(po, (MT_FUNC)&SENDOVERLAPPED::Func));
					break;
				}
				//关闭连接，释放内存，去除这个客户
				case CMOperator::MClose:
				{
					CLOSEOVERLAPPED* po = (CLOSEOVERLAPPED*)pOverlapped;
					SOCKET tmpSock = po->m_client->m_clntSocket;
					ULONGLONG tick = po->m_client->m_tick;
					closesocket(tmpSock);
					if (po->m_client) delete po->m_client;
					m_mapClients.erase(tick);
					break;
				}
			}
		}
		printf("last error %d\r\n", GetLastError());
		return -1;
	}

	int Screen()
	{
		
		if (lstScreenPcks.size() < 3)
		{
			std::list<CPacket> lst;
			CPacket pack(5);
			cmdProc.DispatchCommand(pack, lst);
			lstScreenPcks.push_back(lst.front());
		}		
		return 0;
	}

public:
	CIocpServer() : m_pool(10) , m_HIOCP(INVALID_HANDLE_VALUE)
	{ 
		
		InitEnv();
		CreateSocket();
		m_HIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
		TRACE("last error : %d", GetLastError());
		if (m_HIOCP == NULL)
		{
			printf("IOCP创建失败\r\n");
		}
		if (!CreateIoCompletionPort((HANDLE)m_servSocket, m_HIOCP, 0, 0))
		{
			printf("IOCP绑定失败\r\n");
			return;
		}
	}

	~CIocpServer()
	{
		ResdEnv();
		PostQueuedCompletionStatus(m_HIOCP, -1, -1, NULL);
		std::map<ULONGLONG, CMClient*>::iterator it = m_mapClients.begin();
		for (; it != m_mapClients.end(); it++)
		{
			delete it->second;
		}
		m_mutex.lock();
		m_mapClients.clear();
		m_mutex.unlock();

		lstScreenPcks.clear();
	}

	void StartServer()
	{
		m_pool.Invoke();
		m_pool.DispatchWork(CMWork(this, (MT_FUNC)&CIocpServer::IocpMain));
		NewAccept();
	}

	void NewAccept()
	{
		m_mutex.lock();
		CMClient* pClient = new CMClient(this,m_HIOCP);
		m_mapClients.insert(std::pair<ULONGLONG, CMClient*>(pClient->m_tick,pClient));
		AcceptEx(m_servSocket, pClient->m_clntSocket,
			pClient->m_accept->m_buffer.data(),0,
			sizeof(sockaddr_in) + 16,sizeof(sockaddr_in) + 16,
			&pClient->m_accept->m_received,&pClient->m_accept->m_overlapped);
		m_mutex.unlock();

	}

	void IocpBindSocket(SOCKET sock)
	{
		if (!CreateIoCompletionPort((HANDLE)sock, m_HIOCP, 0, 0))
		{
			TRACE("IOCP绑定失败\r\n");
			return;
		}
	}

	void InitEnv()
	{
		WSADATA wsaData;
		int _ = WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	void ResdEnv()
	{
		WSACleanup();
	}

	void CreateSocket(std::string ip = "0.0.0.0", short port = 6888)
	{
		m_servSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_servSocket == INVALID_SOCKET)
		{
			printf("服务器套接字创建失败\r\n");
		}
		//assert(m_servSocket != INVALID_SOCKET);

		sockaddr_in serv_addr{};
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
		serv_addr.sin_port = htons(port);

		int ret = 0;

		if ((ret = bind(m_servSocket, (sockaddr*)&serv_addr, sizeof(serv_addr))) == SOCKET_ERROR)
		{
			printf("服务器套接字绑定失败\r\n");
		}

		if (((ret = listen(m_servSocket, 5))) == SOCKET_ERROR)
		{
			printf("服务器套接字监听失败\r\n");
		}
	}
};

template<CMOperator op>
inline int AcceptOverlapped<op>::Func()
{
	//等待下一个客户连接
	m_client->m_iocpServer->NewAccept();
	//解析链接过来的用户的地址和端口0
	sockaddr_in* pladdr = NULL;
	sockaddr_in* praddr = NULL;
	INT laddrLen = 0;
	INT raddrLen = 0;
	GetAcceptExSockaddrs(
		m_client->m_accept->m_buffer.data(), m_client->m_accept->m_buffer.size(),
		sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
		(sockaddr**)&pladdr, &laddrLen, (sockaddr**)&praddr, &raddrLen);
	memcpy(&m_client->m_laddr, pladdr, sizeof(sockaddr_in));
	memcpy(&m_client->m_raddr, praddr, sizeof(sockaddr_in));
	//接收输入
	DWORD flag = 0;
	WSARecv(m_client->m_clntSocket, &m_client->m_recv->m_wsaBuf, 1,
		&m_client->m_recv->m_received, &flag, &m_client->m_recv->m_overlapped, NULL);
	return -1;
}

template<CMOperator op>
inline int RecvOverlapped<op>::Func()
{
	//TODO:接收命令
	int readLen = recv(m_client->m_clntSocket, m_client->m_recv->m_buffer.data(), m_client->m_recv->m_buffer.size(), 0);
	//解析命令
	DWORD len = readLen;
	CPacket pack((BYTE*)m_client->m_recv->m_buffer.data(), len);
	//分派命令
	cmdProc.DispatchCommand(pack, m_client->m_send->lstSendPacks);
	//TODO:回复数据->(WSASend)
	if (m_client->m_send->lstSendPacks.size() > 0)
	{
		WSASend(m_client->m_clntSocket, &m_client->m_send->m_wsaBuf, 1,
			&m_client->m_send->m_received, 0, &m_client->m_send->m_overlapped, NULL);
	}
	return -1;
}

template<CMOperator op>
inline int SendtOverlapped<op>::Func()
{
	//一次性发完数据包
	while (m_client->m_send->lstSendPacks.size() > 0)
	{
		int ret = send(m_client->m_clntSocket,
			(char*)	m_client->m_send->lstSendPacks.front().Data(),
					m_client->m_send->lstSendPacks.front().Size(), 0);
		m_client->m_send->lstSendPacks.pop_front();
	}
	//关闭连接
	PostQueuedCompletionStatus(m_client->m_iocp, 1, 1, &m_client->m_close->m_overlapped);
	return -1;
}
