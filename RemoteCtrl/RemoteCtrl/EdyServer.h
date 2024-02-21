#pragma once
#include "EdyThread.h"
#include "CQueue.h"
#include <map>
#include <MSWSock.h>
#include <list>

enum EdyOperator{
	ENone,
	EAccept,
	ERecv,
	ESend,
	EError
};

class EdyServer;
class EdyClient;
typedef std::shared_ptr<EdyClient> PCLIENT;

class EdyOverlapped {
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	std::vector<char> m_buffer;
	ThreadWorker m_worker; // 处理函数
	EdyServer* m_server; // 服务器对象
	EdyClient* m_client;	 //对应的客户端
	WSABUF m_wsabuffer; 
	virtual ~EdyOverlapped() {
		m_buffer.clear();
	}
};

template<EdyOperator>class AcceptOverlapped;
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;

template<EdyOperator>class RecvOverlapped;
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;

template<EdyOperator>class SendOverlapped;
typedef SendOverlapped<ESend> SENDOVERLAPPED;



class EdyClient : public ThreadFuncBase{
public:
	EdyClient();
	~EdyClient() {
		m_buffer.clear();
		closesocket(m_sock);
		m_recv.reset();
		m_send.reset();
		m_overlapped.reset();
		m_vecSend.Clear();
	}
	void SetOverlapped(PCLIENT& ptr);

	operator SOCKET() {
		return m_sock;
	}
	operator PVOID() {
		return &m_buffer[0];
	}
	operator LPOVERLAPPED();

	operator LPDWORD() {
		return &m_received;
	}

	LPWSABUF RecvWSABuffer();
	LPWSAOVERLAPPED RecvOverlapped();

	LPWSABUF SendWSABuffer();
	LPWSAOVERLAPPED SendOverlapped();

	DWORD& flags() {
		return m_flags;
	}
	sockaddr_in* GetLocalAddr() {
		return &m_laddr;
	}
	sockaddr_in* GetRemoteAddr() {
		return &m_raddr;
	}

	size_t GetBufferSize() const {
		return m_buffer.size();
	}
	int Recv();

	int Send(void* buffer, size_t nSize);
	int SendData(std::vector<char>& data);
private:

	SOCKET m_sock;
	DWORD m_received;
	DWORD m_flags;
	std::shared_ptr<ACCEPTOVERLAPPED> m_overlapped;
	std::shared_ptr<RECVOVERLAPPED> m_recv;
	std::shared_ptr<SENDOVERLAPPED> m_send;
	std::vector<char> m_buffer;
	size_t m_used; // 已经使用的缓冲区大小
	sockaddr_in m_laddr; //本地地址
	sockaddr_in m_raddr; //远程地址
	bool m_isbusy;
	EdySendQueue<std::vector<char>> m_vecSend; //发送数据队列
};

template<EdyOperator>
class AcceptOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	AcceptOverlapped();
	int AcceptWorker();};

template<EdyOperator>
class RecvOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	RecvOverlapped();
	int RecvWorker() {
		int ret = m_client->Recv();
		return ret;
	}

};

template<EdyOperator>
class SendOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	SendOverlapped();
	int SendWorker() {
		
		return -1;
	}

};
typedef SendOverlapped<ESend> SENDOVERLAPPED;


template<EdyOperator>
class ErrorOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	ErrorOverlapped() :m_operator(EError), m_worker(this, &ErrorOverlapped::ErrorWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024);
	}
	int ErrorWorker() {
		//TODO
		return -1;
	}
};
typedef ErrorOverlapped<EError> ERROROVERLAPPED;

class EdyServer : public ThreadFuncBase
{
public:
	EdyServer(const std::string& ip = "0.0.0.0", short port = 9527) :m_pool(10) {
		m_hIOCP = INVALID_HANDLE_VALUE;
		m_sock = INVALID_SOCKET;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htonl(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}
	~EdyServer();

	bool StartService();
	bool NewAccept();
	void BindNewSocket(SOCKET s);
private:
	void CreateSocket();

	int threadIocp();
private:
	EdyThreadPool m_pool;
	HANDLE m_hIOCP;
	SOCKET m_sock;
	sockaddr_in m_addr;
	std::map<SOCKET, std::shared_ptr<EdyClient>> m_client;
};