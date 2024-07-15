#pragma once
#include "framework.h"
#include "Common.h"
#include "MThread.h"
#include <string>
#include <vector>
#include <atomic>
#include <map>

class UDPPassClient : public CMFuncBase
{
private:
	MUserInfo						m_currentUser;			//自己的信息
	std::map<long long, MUserInfo>	m_mapAddrs;				//在线的其他人的信息
	sockaddr_in						m_udpAddr;
	sockaddr_in						m_tcpAddr;
	SOCKET							m_udpSock;
	SOCKET							m_tcpSock;
	CMThreadPool					m_thpool;
	std::atomic<bool>				m_stop;
	HWND							m_hWnd;
	CPacket							m_udpConectPack;
private:
	int ThreadTcpProc();
	int ThreadUdpProc();
	int ThreadUDPPass();
	/// <summary>
/// 初始化网络环境
/// </summary>
	void InitSockEnv()
	{
		//初始化网络库
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("初始化网络库失败(错误码:%d 错误:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"初始化网络库失败");
			exit(-1);
		}
	}
	/// <summary>
	/// 释放网路环境
	/// </summary>
	void DesSockEnv()
	{
		WSACleanup();
	}
	int KeepOnline();
	
public:
	UDPPassClient(const std::string& ip, short tcpPort, short udpPort);
	~UDPPassClient();
	int Invoke(HWND hWnd);
	void DealUdp(CPacket& pack, sockaddr_in& addr);
	void DealTcp(CPacket& pack);
	std::map<long long, MUserInfo>& GetMapAddrs();
	void RequestConnect(long long id);
	void SentToBeCtrl();
};

