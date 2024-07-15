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
	//TCP服务器主要逻辑
	int ThreadTcpProc();
	//UDP服务器主要逻辑
	int ThreadUdpProc();
	//处理每个TCP客户
	int ThreadTcpClnt(void* arg);
	//测试用户是否在线
	int TestOnline();
	//获得所有用户地址信息(返回值：包的命令位-1表示错误)
	CPacket GetSendAddr(long long id);
	//根据socket删除信息
	void EraseAddrBySocket(int sock);
public:
	UDPPassNetWork(const std::string& ip, short tcpPort, short udpPort);
	~UDPPassNetWork();
	//开启
	int Invoke();
	//处理用户请求
	int DealUdp(CPacket& pack, sockaddr_in& clnt_addr);
	int DealTcp(CPacket& pack,int sock);
	//给所有用户发送地址
	int SendAddrs();
};

