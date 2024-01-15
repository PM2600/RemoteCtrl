#pragma once

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"

class CServerSocket
{ 
public:
	static CServerSocket* getInstance() {
		// 静态函数没有this指针，所以无法直接访问成员变量，需要将成员变量也设置为静态变量
		if (m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}

	bool InitSocket() {
		if (m_server == -1) {
			printf("socket error\n");
			return false;
		}

		SOCKADDR_IN serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_port = htons(8888);
		serv_adr.sin_addr.s_addr = INADDR_ANY;

		if (bind(m_server, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == -1) {
			printf("bind error\n");
			return false;
		}

		if (listen(m_server, 1) == -1) {
			printf("listen error\n");
			return false;
		}
		return true;
	}

	bool AcceptClient() {
		SOCKADDR_IN cli_adr;
		int cli_sz = sizeof(cli_adr);
		m_client = accept(m_server, (SOCKADDR*)&cli_adr, &cli_sz);
		if (m_client == -1) {
			printf("socket error\n");
			return false;
		}
		return true;
	}

	int DealCommand() {
		if (m_client == -1)
			return -1;
		char buffer[1024] = "";
		while (1) {
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0) {
				return -1;
			}
			// TODO: 处理命令

		}
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}



private:
	SOCKET m_server;
	SOCKET m_client;


	CServerSocket& operator=(const CServerSocket& ss) {}

	CServerSocket(const CServerSocket& ss) {
		m_server = ss.m_server;
		m_client = ss.m_client;
	}

	CServerSocket(){
		m_client = INVALID_SOCKET;

		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置"), _T("错误!"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_server = socket(PF_INET, SOCK_STREAM, 0);
	}

	~CServerSocket(){
		closesocket(m_server);
		WSACleanup();
	}

	BOOL InitSockEnv() {
		// 初始化网络库
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CServerSocket* m_instance;

	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};

extern CServerSocket server;

