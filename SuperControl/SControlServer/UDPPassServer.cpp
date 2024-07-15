#include "pch.h"
#include "Common.h"
#include "UDPPassServer.h"
#include "CmdProcessor.h"

extern CCmdProcessor cmdProc;

int UDPPassServer::ThreadTcpProc()
{
	//连接服务器(TCP)
	int ret = connect(m_tcpSock, (sockaddr*)&m_tcpAddr, sizeof(sockaddr_in));
	if (ret == -1)
	{
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, GetLastError(), strerror(errno));
		return -1;
	}
	//向服务器发个包，表示我上线了
	CPacket pack(101, (BYTE*)&m_currentUser, sizeof(MUserInfo));
	send(m_tcpSock, (char*)pack.Data(), pack.Size(), 0);

	//
	while (true)
	{
		char buf[1024]{};
		//获取数据
		int ret = recv(m_tcpSock, buf, 1024, 0);
		if (ret <= 0)
		{
			closesocket(m_tcpSock);
			break;
		}
		//解析数据
		DWORD len = (int)ret;
		CPacket pack = CPacket((BYTE*)buf, len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//处理数据
		DealTcp(pack);
	}

	return -1;
}

int UDPPassServer::ThreadUdpProc()
{
	//向服务器发个包，表示我上线了
	CPacket pack(101,(BYTE*)&m_currentUser.id,sizeof(m_currentUser.id));
	sendto(m_udpSock, (char*)pack.Data(), pack.Size(),
		0, reinterpret_cast<sockaddr*>(&m_udpAddr), sizeof(sockaddr_in));
	//收取一个服务器发来的包，建立起连接
	char buf[1024]{};
	sockaddr_in serv_addr{};
	int serv_addr_len = sizeof(serv_addr);
	recvfrom(m_udpSock, buf, sizeof(buf), 0, (sockaddr*)&serv_addr, &serv_addr_len);
	//等待服务器，发来数据
	while (true)
	{
		char buf[1024]{};
		sockaddr_in addr{};
		int addr_len{ sizeof(addr) };
		//获取数据
		int ret = recvfrom(m_udpSock, buf, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
		//解析数据
		DWORD len = (int)ret;
		CPacket pack = CPacket(reinterpret_cast<byte*>(buf), len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//处理数据
		DealUdp(pack, addr);
	}
	return -1;
}

int UDPPassServer::KeepOnline()
{
	CPacket pack(103);
	while (true)
	{
		sendto(m_udpSock, (char*)pack.Data(), pack.Size(),
			0, reinterpret_cast<sockaddr*>(&m_udpAddr), sizeof(sockaddr_in));
		Sleep(1000);
	}
	return -1;
}

int UDPPassServer::ThreadUDPPass()
{
	CPacket pack(123, (BYTE*)"ping", 4);
	MUserInfo* pMInfo = (MUserInfo*)m_udpConectPack.sData.c_str();
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(pMInfo->ip);
	addr.sin_port = htons(pMInfo->port);
	for (int i = 0; i < 3; i++)
	{
		sendto(m_udpSock, (char*)pack.Data(), pack.Size(), 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	}

	return -1;
}

UDPPassServer::UDPPassServer(const std::string& ip, short tcpPort, short udpPort) : m_tcpAddr(), m_udpAddr(), m_tcpSock(-1), m_udpSock(-1) ,m_thpool(5)
{
	//配置端口地址(TCP)
	memset(&m_tcpAddr, 0, sizeof(m_tcpAddr));
	m_tcpAddr.sin_family = AF_INET;
	m_tcpAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_tcpAddr.sin_port = htons(tcpPort);
	//配置端口地址(UDP)
	memset(&m_udpAddr, 0, sizeof(m_udpAddr));
	m_udpAddr.sin_family = AF_INET;
	m_udpAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_udpAddr.sin_port = htons(udpPort);
}

UDPPassServer::~UDPPassServer()
{
	closesocket(m_tcpSock);
	closesocket(m_udpSock);
}

int UDPPassServer::Invoke()
{
	//创建套接字(UDP)
	m_tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_tcpSock == -1)
	{
		printf("%s(%d):%s socket error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//创建套接字(UDP)
	m_udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_udpSock == -1)
	{
		printf("%s(%d):%s socket error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}

	//绑定套接字(UDP)
	sockaddr_in localAddr{};
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = 0;
	localAddr.sin_port = 23456;//[=======PORT======]
	if (bind(m_udpSock, (sockaddr*)&localAddr, sizeof(sockaddr_in)) == -1)
	{
		printf("%s(%d):%s socket error udp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}

	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassServer::ThreadUdpProc));
	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassServer::ThreadTcpProc));
	m_thpool.Invoke();
	return 1;
}

void UDPPassServer::DealUdp(CPacket& pack, sockaddr_in& addr)
{
	std::list<CPacket> lstSends;
	cmdProc.DispatchCommand(pack,lstSends);
	while (lstSends.size() > 0)
	{
		PFILEINFO pFileInfo = (PFILEINFO)lstSends.front().sData.c_str();
		TRACE("* %s\r\n", pFileInfo->data.name);
		sendto(m_udpSock, (char*)lstSends.front().Data(), lstSends.front().Size(),0, (sockaddr*)&addr,sizeof(sockaddr_in));
		lstSends.pop_front();
		Sleep(10);
	}
}

void UDPPassServer::DealTcp(CPacket& pack)
{
	switch (pack.nCmd)
	{
		case 102://服务器发来在线用户的地址信息
		{
			if (pack.sData.size() == 0)
			{
				break;
			}
			m_vecSockAddrs.resize(pack.sData.size() / sizeof(MUserInfo));
			memcpy(m_vecSockAddrs.data(), pack.sData.c_str(), pack.sData.size());
			MUserInfo& mInfo = m_vecSockAddrs.at(0);
			WCHAR wideIp[32]{};
			MultiByteToWideChar(CP_ACP, 0, mInfo.ip, 16, wideIp, 32);
			break;
		}
		case 105://服务器发来数据，叫我和指定用户连接
		{
			m_udpConectPack = pack;
			m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassServer::ThreadUDPPass));
			break;
		}
	}
}
