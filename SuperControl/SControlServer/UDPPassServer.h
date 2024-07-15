#pragma once
#include <vector>
#include "Common.h"
#include "MThread.h"
class UDPPassServer : public CMFuncBase
{
private:
	MUserInfo				m_currentUser;
	std::vector<MUserInfo>	m_vecSockAddrs;
	sockaddr_in				m_udpAddr;
	sockaddr_in				m_tcpAddr;
	int						m_tcpSock;
	int						m_udpSock;
	CMThreadPool			m_thpool;
	CPacket					m_udpConectPack;
private:
	int ThreadTcpProc();
	int ThreadUdpProc();
	int KeepOnline();
	int ThreadUDPPass();
public:
	UDPPassServer(const std::string& ip, short tcpPort, short udpPort);
	~UDPPassServer();
	int Invoke();
	void DealUdp(CPacket& pack, sockaddr_in& addr);
	void DealTcp(CPacket& pack);
};

