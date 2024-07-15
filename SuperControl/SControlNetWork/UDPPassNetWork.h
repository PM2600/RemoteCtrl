#pragma once

#include <cstdio>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include <sys/time.h>
#include <map>
#include <mutex>
#include "Common.h"
#include "MThread.h"
class UDPPassNetWork : public CMFuncBase
{
private:
	std::map<long long, MUserInfo>	m_mapAddrs;
	sockaddr_in						m_udpServAddr;
	sockaddr_in						m_tcpServAddr;
	int								m_udpSock;
	int								m_tcpSock;
	CMThreadPool					m_thpool;
	std::atomic<bool>				m_stop;
	std::mutex						m_mutex;
private:
	//TCP��������Ҫ�߼�
	int ThreadTcpProc();
	//UDP��������Ҫ�߼�
	int ThreadUdpProc();
	//����ÿ��TCP�ͻ�
	int ThreadTcpClnt(void* arg);
	//�����û��Ƿ�����
	int TestOnline();
	//��������û���ַ��Ϣ(����ֵ����������λ-1��ʾ����)
	CPacket GetSendAddr(long long id);
	//����socketɾ����Ϣ
	void EraseAddrBySocket(int sock);
public:
	UDPPassNetWork(const std::string& ip, short tcpPort, short udpPort);
	~UDPPassNetWork();
	//����
	int Invoke();
	//�����û�����
	int DealUdp(CPacket& pack, sockaddr_in& clnt_addr);
	int DealTcp(CPacket& pack,int sock);
	//�������û����͵�ַ
	int SendAddrs();
};

