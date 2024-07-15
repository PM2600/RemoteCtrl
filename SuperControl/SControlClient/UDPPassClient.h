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
	MUserInfo						m_currentUser;			//�Լ�����Ϣ
	std::map<long long, MUserInfo>	m_mapAddrs;				//���ߵ������˵���Ϣ
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
/// ��ʼ�����绷��
/// </summary>
	void InitSockEnv()
	{
		//��ʼ�������
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(1, 1), &wsaData);
		if (ret != 0)
		{
			TRACE("��ʼ�������ʧ��(������:%d ����:%s)\r\n", ret, GetErrInfo(ret));
			AfxMessageBox(L"��ʼ�������ʧ��");
			exit(-1);
		}
	}
	/// <summary>
	/// �ͷ���·����
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

