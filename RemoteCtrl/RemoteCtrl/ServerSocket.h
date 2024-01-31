#pragma once

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include <list>
#include "Packet.h"


typedef void(*SOCKET_CALLBACK)(void*, int, std::list<CPacket>&, CPacket&);
	 
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

	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527) {
		int ret = InitSocket(port);
		if (ret == false)
			return -1;
		std::list<CPacket> lstPacket;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3) {
					return -2;
				}
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(m_arg, ret, lstPacket, m_packet);
				while (lstPacket.size() > 0) {
					Send(lstPacket.front());
					lstPacket.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}

protected:
	bool InitSocket(short port) {
		if (m_server == -1) {
			printf("socket error\n");
			return false;
		}
		SOCKADDR_IN serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_port = htons(port);
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
		TRACE("enter AcceptClient\r\n");
		SOCKADDR_IN cli_adr;
		int cli_sz = sizeof(cli_adr);
		m_client = accept(m_server, (SOCKADDR*)&cli_adr, &cli_sz);
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1) {
			printf("socket error\n");
			return false;
		}
		return true;
	}

#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1)
			return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		static size_t index = 0;
		while (1) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				delete[] buffer;
				return -1;
			}
			TRACE("recv %d\r\n", len);
			index += len;
			len = index; // ??? 
			m_packet = CPacket((BYTE*)buffer, len);
			TRACE("len=%d\r\n", len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[] buffer;
				TRACE("here\r\n");
				return m_packet.sCmd;
			}
		}
		delete[] buffer;
		return -1;
	}

	bool Send(const char* pData, size_t nSize) {
		if (m_client == -1)
			return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)
			return false;
		//Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if (((m_packet.sCmd >= 2) && (m_packet.sCmd <= 4)) || m_packet.sCmd == 9) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}

private:
	SOCKET_CALLBACK m_callback;
	void* m_arg;
	SOCKET m_server;
	SOCKET m_client;
	CPacket m_packet;

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

//extern CServerSocket server;

