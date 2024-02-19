#pragma once
#include "EdyThread.h"
#include <map>

class EdyClient {

};

enum EdyOperator{
	ENone,
	EAccept,
	ERecv,
	ESend,
	EError
};

class EdyOverlapped {
public:
	OVERLAPPED m_overlapped;
	DWORD m_operator;
	std::vector<char> m_buffer;
	ThreadWorker m_worker; // ´¦Àíº¯Êý

};

template<EdyOperator>
class AcceptOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	AcceptOverlapped() :m_operator(EAccept), m_worker(this, &AcceptOverlapped::AcceptWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024);
	}
	int AcceptWorker() {
		//TODO
	}
};
typedef AcceptOverlapped<EAccept> ACCEPTOVERLAPPED;


template<EdyOperator>
class RecvOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	RecvOverlapped() :m_operator(ERecv), m_worker(this, &RecvOverlapped::RecvWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024 * 256);
	}
	int RecvWorker() {
		//TODO
	}
};
typedef RecvOverlapped<ERecv> RECVOVERLAPPED;


template<EdyOperator>
class SendOverlapped :public EdyOverlapped, ThreadFuncBase {
public:
	SendOverlapped() :m_operator(ESend), m_worker(this, &SendOverlapped::SendWorker) {
		memset(&m_overlapped, 0, sizeof(m_overlapped));
		m_buffer.resize(1024 * 256);
	}
	int SendWorker() {
		//TODO
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
	}
};
typedef ErrorOverlapped<EError> ERROROVERLAPPED;


class EdyServer : public ThreadFuncBase
{
public:
	EdyServer(const std::string& ip = "0.0.0.0", short port = 9527) :m_pool(10) {
		m_hIOCP = INVALID_HANDLE_VALUE;
		m_sock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		int opt = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htonl(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if (bind(m_sock, (sockaddr*)&addr, sizeof(sockaddr_in)) == -1) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return;
		}
		if (listen(m_sock, 3) == -1) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return;
		}

		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
		if (m_hIOCP == NULL) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hIOCP = INVALID_HANDLE_VALUE;
			return;
		}
		CreateIoCompletionPort((HANDLE)m_sock, m_hIOCP, (ULONG_PTR)this, 0);
		m_pool.DispatchWorker(ThreadWorker(this, (FUNCTYPE)&EdyServer::threadIocp));

	}
	~EdyServer(){}
private:
	int threadIocp() {
		DWORD tranferred = 0;
		ULONG_PTR CompletionKey = 0;
		OVERLAPPED* lpOverlapped = NULL;
		if (GetQueuedCompletionStatus(m_hIOCP, &tranferred, &CompletionKey, &lpOverlapped, INFINITY)) {
			if (tranferred > 0 && CompletionKey != 0) {
				EdyOverlapped* pOverlapped = CONTAINING_RECORD(lpOverlapped, EdyOverlapped, m_overlapped);
				switch (pOverlapped->m_operator) {
				case EAccept:
					ACCEPTOVERLAPPED* pOver = (ACCEPTOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
					break;
				case ERecv:
					RECVOVERLAPPED* pOver = (RECVOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
					break;
				case ESend:
					SENDOVERLAPPED* pOver = (SENDOVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
					break;
				case EError:
					ERROROVERLAPPED* pOver = (ERROROVERLAPPED*)pOverlapped;
					m_pool.DispatchWorker(pOver->m_worker);
					break;
				}		
			}
			else {
				return -1;
			}
		}
		return 0;
	}
private:
	EdyThreadPool m_pool;
	HANDLE m_hIOCP;
	SOCKET m_sock;
	std::map<SOCKET, std::shared_ptr<EdyClient*>> m_client;
};

