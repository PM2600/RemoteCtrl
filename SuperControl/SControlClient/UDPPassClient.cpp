#include "pch.h"
#include "UDPPassClient.h"

int UDPPassClient::ThreadTcpProc()
{
	//���ӷ�����(TCP)
	int ret = connect(m_tcpSock, (sockaddr*)&m_tcpAddr, sizeof(sockaddr_in));
	if (m_tcpSock == -1)
	{
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return -1;
	}
	//�����������������ʾ��������
	CPacket pack(101, (BYTE*)&m_currentUser, sizeof(MUserInfo));
	send(m_tcpSock, (char*)pack.Data(), pack.Size(), 0);

	//
	while (!m_stop)
	{
		char buf[1024]{};
		//��ȡ����
		int ret = recv(m_tcpSock, buf, 1024, 0);
		if (ret <= 0)
		{
			closesocket(m_tcpSock);
			break;
		}
		//��������
		int len = (int)ret;
		CPacket pack = CPacket((BYTE*)buf, len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//��������
		DealTcp(pack);
	}

	return -1;
}

int UDPPassClient::ThreadUdpProc()
{
	//�����������������ʾ��������
	CPacket pack(101, (BYTE*)&m_currentUser.id, sizeof(m_currentUser.id));
	sendto(m_udpSock, (char*)pack.Data(), pack.Size(),
		0, reinterpret_cast<sockaddr*>(&m_udpAddr), sizeof(sockaddr_in));
	//��ȡһ�������������İ�������������
	char buf[1024]{};
	sockaddr_in serv_addr{};
	int serv_addr_len = sizeof(serv_addr);
	recvfrom(m_udpSock, buf, sizeof(buf), 0, (sockaddr*)&serv_addr, &serv_addr_len);
	//�ȴ�����������������
	while (!m_stop)
	{
		char buf[1024]{};
		sockaddr_in addr{};
		int addr_len{ sizeof(addr) };
		//��ȡ����
		int ret = recvfrom(m_udpSock, buf, 1024, 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
		//��������
		int len = (int)ret;
		CPacket pack = CPacket((BYTE*)buf, len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//��������
		DealUdp(pack, addr);
	}
	return -1;
}

int UDPPassClient::ThreadUDPPass()
{
	CPacket pack(123, (BYTE*)"ping", 4);
	MUserInfo* pMInfo = (MUserInfo*)m_udpConectPack.sData.c_str();
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(pMInfo->ip);
	addr.sin_port = htons(pMInfo->port);
	for (int i = 0; i < 10; i++)
	{
		sendto(m_udpSock, (char*)pack.Data(), pack.Size(), 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	}

	return -1;
}


int UDPPassClient::KeepOnline()
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

void UDPPassClient::SentToBeCtrl()
{
	if (m_udpConectPack.nCmd != -1)
	{
		MUserInfo* pMInfo = (MUserInfo*)m_udpConectPack.sData.c_str();
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(pMInfo->ip);
		addr.sin_port = htons(pMInfo->port);
		char multiPath[]{ "hello" };
		CPacket pack(2, (BYTE*)multiPath, strlen(multiPath));
		sendto(m_udpSock, (char*)pack.Data(), pack.Size(), 0, (sockaddr*)&addr, sizeof(sockaddr_in));

	}

}

UDPPassClient::UDPPassClient(const std::string& ip, short tcpPort, short udpPort) : m_tcpAddr(), m_udpAddr(), m_tcpSock(-1), m_udpSock(-1), m_thpool(5)
{
	InitSockEnv();
	m_stop = false;
	//���ö˿ڵ�ַ(TCP)
	memset(&m_udpAddr, 0, sizeof(m_tcpAddr));
	m_tcpAddr.sin_family = AF_INET;
	m_tcpAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_tcpAddr.sin_port = htons(tcpPort);
	//���ö˿ڵ�ַ(UDP)
	memset(&m_udpAddr, 0, sizeof(m_udpAddr));
	m_udpAddr.sin_family = AF_INET;
	m_udpAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_udpAddr.sin_port = htons(udpPort);
	memcpy(m_currentUser.ip, ip.c_str(), ip.size());
}

UDPPassClient::~UDPPassClient()
{
	m_stop = true;
	DesSockEnv();
}

int UDPPassClient::Invoke(HWND hWnd)
{
	m_hWnd = hWnd;
	//�����׽���(TCP)
	m_tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_tcpSock == -1)
	{
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//�����׽���(UDP)
	m_udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_udpSock == -1)
	{
		printf("%s(%d):%s socket error udp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//���׽���(UDP)
	sockaddr_in localAddr{};
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = 0;
	localAddr.sin_port = 12345;//[=======PORT======]
	if (bind(m_udpSock, (sockaddr*)&localAddr, sizeof(sockaddr_in)) == -1)
	{
		printf("%s(%d):%s socket error udp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}

	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassClient::ThreadUdpProc));
	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassClient::ThreadTcpProc));
	m_thpool.Invoke();
	return 1;
}

void UDPPassClient::DealUdp(CPacket& pack, sockaddr_in& addr)
{

	int a = 0;

	if (pack.nCmd == 2)
	{
		TRACE("udp��Ϣ:%s\r\n", pack.sData.c_str());
		MessageBox(NULL, _T("hello"), _T("��ȡudp��Ϣ"), MB_OK);
	}
}

void UDPPassClient::DealTcp(CPacket& pack)
{
	switch (pack.nCmd)
	{
	case 102:
	{
		if (pack.sData.size() == 0)
		{
			m_mapAddrs.clear();
			SendMessage(m_hWnd, (WM_USER + 10), 1, NULL);
			break;
		}
		std::vector<MUserInfo> m_vecSockAddrs;
		m_vecSockAddrs.resize(pack.sData.size() / sizeof(MUserInfo));
		memcpy(m_vecSockAddrs.data(), pack.sData.c_str(), pack.sData.size());
		//�����Լ���Ϣ
		memcpy(&m_currentUser, &m_vecSockAddrs.at(0), sizeof(MUserInfo));
		//������������Ϣ
		for (int i = 1; i < m_vecSockAddrs.size(); i++)
		{
			m_mapAddrs.insert(std::pair<long long, MUserInfo>(m_vecSockAddrs.at(i).id, m_vecSockAddrs.at(i)));
		}
		SendMessage(m_hWnd, (WM_USER + 10), NULL, NULL);
		break;
	}
	case 105://�������������ݣ����Һ�ָ���û�����
	{
		m_udpConectPack = pack;
		m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassClient::ThreadUDPPass));
		break;
	}

	}
}

std::map<long long, MUserInfo>& UDPPassClient::GetMapAddrs()
{
	return m_mapAddrs;
}

void UDPPassClient::RequestConnect(long long id)
{
	std::map<long long, MUserInfo>::iterator it = m_mapAddrs.find(id);
	if (it == m_mapAddrs.end()) return;

	ConnectIds ids{ m_currentUser.id,id };
	CPacket pack(104, (BYTE*)&ids, sizeof(ids));

	send(m_tcpSock, (char*)pack.Data(), pack.Size(), 0);
}
