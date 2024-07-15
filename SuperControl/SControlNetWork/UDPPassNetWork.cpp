#include "UDPPassNetWork.h"
#include "Common.h"
#include <list>

int UDPPassNetWork::ThreadTcpProc()
{
	//接入客户端
	while (!m_stop)
	{
		sockaddr_in clnt_addr{};
		socklen_t   clnt_addr_len{ sizeof(clnt_addr) };
		int clntSock = accept(m_tcpSock, (sockaddr*)(&clnt_addr), &clnt_addr_len);

		m_thpool.DispatchWork(CMWork(this, (MT_FUNC2)&UDPPassNetWork::ThreadTcpClnt, reinterpret_cast<void*>(clntSock)));
	}

	return -1;
}

int UDPPassNetWork::ThreadUdpProc()
{
	//接入客户端
	while (!m_stop)
	{
		char        buf[1024]{};
		sockaddr_in clnt_addr{};
		socklen_t   clnt_addr_len{ sizeof(clnt_addr) };

		//获取数据
		ssize_t ret = recvfrom(m_udpSock, buf, 1024, 0, (sockaddr*)(&clnt_addr), &clnt_addr_len);	
		
		//解析数据
		int len = (int)ret;
		CPacket pack = CPacket((unsigned char*)buf, len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//处理数据
		DealUdp(pack, clnt_addr);
	}
	return -1;
}

int UDPPassNetWork::ThreadTcpClnt(void* arg)
{
	int sock = (int)(long long)arg;

	while (true)
	{
		char        buf[1024]{};
		//获取数据
		ssize_t ret = recv(sock, buf, sizeof(buf), 0);
		if (ret <= 0)
		{
			close(sock);
			EraseAddrBySocket(sock);
			SendAddrs();
			break;
		}
		//解析数据
		int len = (int)ret;
		CPacket pack = CPacket((unsigned char*)buf, len);
		if (len <= 0)
		{
			printf("%s(%d):%s packet parse error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
			continue;
		}
		//处理数据
		DealTcp(pack,sock);
	}

	return -1;
}

UDPPassNetWork::UDPPassNetWork(const std::string& ip, short tcpPort, short udpPort) 
	: m_udpServAddr()
	, m_tcpServAddr()
	, m_udpSock(-1) 
	, m_tcpSock(-1)
	, m_thpool(10)
{
	m_stop = true;
	//配置端口地址(TCP)
	memset(&m_tcpServAddr, 0, sizeof(m_tcpServAddr));
	m_tcpServAddr.sin_family = AF_INET;
	m_tcpServAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	m_tcpServAddr.sin_port = htons(tcpPort);
	//配置端口地址(UDP)
	memset(&m_udpServAddr, 0, sizeof(m_udpServAddr));
	m_udpServAddr.sin_family = AF_INET;
	m_udpServAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_udpServAddr.sin_port = htons(udpPort);
	//
}

UDPPassNetWork::~UDPPassNetWork()
{
	m_stop = true;

	for (std::map<long long, MUserInfo>::iterator it = m_mapAddrs.begin(); it != m_mapAddrs.end(); it++)
	{
		close(it->second.tcpSock);
	}

	close(m_tcpSock);
	close(m_udpSock);

}

int UDPPassNetWork::Invoke()
{
	m_stop = false;
	//创建套接字(TCP)
	m_tcpSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_tcpSock == -1)
	{
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//绑定(TCP)
	if (bind(m_tcpSock, (struct sockaddr*)&m_tcpServAddr, sizeof(struct sockaddr_in)) == -1)
	{
		close(m_tcpSock);
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//监听(TCP)
	if (listen(m_tcpSock, 5) == -1)
	{
		close(m_tcpSock);
		printf("%s(%d):%s socket error tcp (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}

	//创建套接字(UDP)
	m_udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_udpSock == -1)
	{
		printf("%s(%d):%s socket error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//绑定(UDP)
	if (bind(m_udpSock, (struct sockaddr*)&m_udpServAddr, sizeof(struct sockaddr_in)) == -1)
	{
		close(m_udpSock);
		printf("%s(%d):%s socket error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
		return 0;
	}
	//启动
	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassNetWork::ThreadUdpProc));
	m_thpool.DispatchWork(CMWork(this, (MT_FUNC)&UDPPassNetWork::ThreadTcpProc));
	m_thpool.Invoke();

	return 0;
}

int UDPPassNetWork::DealUdp(CPacket& pack,sockaddr_in& clnt_addr)
{
	switch (pack.nCmd)
	{
		case 101://用户连接上来
		{
			{
				//用户连上来，发了个id过来
				char ip[16]{};
				short port{};
				int intIp = (clnt_addr.sin_addr.s_addr);
				unsigned char* pCharIp = (unsigned char*)&intIp;
				sprintf(ip, "%d.%d.%d.%d", pCharIp[0], pCharIp[1], pCharIp[2], pCharIp[3]);
				port = ntohs(clnt_addr.sin_port);
				long long id = *(long long*)pack.sData.c_str();
				m_mutex.lock();
				std::map<long long, MUserInfo>::iterator find = m_mapAddrs.find(id);
				//根据id找到地址，就修改就行了
				if (find != m_mapAddrs.end())
				{
					memcpy(find->second.ip, ip, 16);
					find->second.port = port;
					SendAddrs();
					printf("(exist)udp online :%s\n", ip);
				}
				//根据id找没到地址，就根据id创建一个
				else
				{
					MUserInfo mInfo(ip, port);
					mInfo.id = id;
					m_mapAddrs.insert(std::pair<long long, MUserInfo>(mInfo.id, mInfo));
					printf("udp online :%s\n", ip);
				}
				m_mutex.unlock();

				//回应一个信息
				CPacket ackPack(101);
				sendto(m_udpSock, ackPack.Data(), ackPack.Size(), 0, (sockaddr*)&clnt_addr, sizeof(sockaddr_in));

			}
			//TODO:通知tcp发地址信息给用户
			
			break;
		}
		case 103://用户发送心跳包（保持在线）
		{
			unsigned long long id = 0;
			memcpy(&id, pack.sData.c_str(), sizeof(long long));
			std::map<long long, MUserInfo>::iterator it = m_mapAddrs.find(id);
			if (it != m_mapAddrs.end())
			{
				//获得当前时间错（毫秒级别）
				struct timeval tv;
				gettimeofday(&tv, NULL);
				it->second.last = tv.tv_sec * 1000 + tv.tv_usec / 1000;
			}
			break;
		}
		case 104://控制端想要发起控制（建立udp穿透）【告诉两个客户端你们可以相互连接了】
		{
			ConnectIds ids;
			memcpy(&ids, pack.sData.c_str(), sizeof(ConnectIds));
			
			std::map<long long, MUserInfo>::iterator it0 = m_mapAddrs.find(ids.id0);
			std::map<long long, MUserInfo>::iterator it1 = m_mapAddrs.find(ids.id1);
			ssize_t ret = 0;
			if ((it0 != m_mapAddrs.end()) && (it1 != m_mapAddrs.end()))
			{
				sockaddr_in addr0{}, addr1{};
				addr0.sin_family = AF_INET;
				addr0.sin_addr.s_addr = inet_addr(it0->second.ip);
				addr0.sin_port = htons(it0->second.port);
				addr1.sin_family = AF_INET;
				addr1.sin_addr.s_addr = inet_addr(it1->second.ip);
				addr1.sin_port = htons(it1->second.port);

				MUserInfo mInfo0(it1->second.ip, it1->second.port);
				MUserInfo mInfo1(it0->second.ip, it0->second.port);
				CPacket sendPack0(105, (unsigned char*)&mInfo0,sizeof(MUserInfo));
				CPacket sendPack1(105, (unsigned char*)&mInfo1, sizeof(MUserInfo));

				
				ret = sendto(m_udpSock, sendPack0.Data(), sendPack0.Size(), 0, (sockaddr*)&addr0, sizeof(sockaddr_in));
				if (ret <= 0) 
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
				ret = sendto(m_udpSock, sendPack1.Data(), sendPack1.Size(), 0, (sockaddr*)&addr1, sizeof(sockaddr_in));
				if (ret <= 0)
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
			}
			else
			{
				CPacket sendPack(106);
				ret = sendto(m_udpSock, sendPack.Data(), sendPack.Size(), 0, (sockaddr*)&clnt_addr, sizeof(sockaddr_in));
				if (ret <= 0)
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
			}
			break;
		}
	}
	return 0;
}

int UDPPassNetWork::DealTcp(CPacket& pack,int sock)
{
	switch (pack.nCmd)
	{
		case 101://用户连接上来了
		{
			MUserInfo mInfo("",0);
			memcpy(&mInfo, pack.sData.c_str(), pack.sData.size());
			mInfo.tcpSock = sock;

			m_mutex.lock();
			std::map<long long, MUserInfo>::iterator find_ = m_mapAddrs.find(mInfo.id);
			//根据id找到地址，就修改就行了
			if (find_ != m_mapAddrs.end())
			{
				MUserInfo tmp = find_->second;
				memcpy(&find_->second, &mInfo, sizeof(MUserInfo));
				memcpy(find_->second.ip, tmp.ip, 16);
				find_->second.port = tmp.port;
				printf("already exist:%s %d\n", mInfo.ip, ntohs(mInfo.port));
				SendAddrs();

			}
			//根据id找没到地址，就根据id创建一个
			else
			{
				m_mapAddrs.insert(std::pair<long long, MUserInfo>(mInfo.id, mInfo));
				printf("map size:%d  mInfo.id:%lld\n", m_mapAddrs.size(), mInfo.id);
			}
			m_mutex.unlock();
			break;
		}
		case 103://用户发送心跳包（保持在线）
		{
			unsigned long long id = 0;
			memcpy(&id, pack.sData.c_str(), sizeof(long long));
			std::map<long long, MUserInfo>::iterator it = m_mapAddrs.find(id);
			if (it != m_mapAddrs.end())
			{
				//获得当前时间错（毫秒级别）
				struct timeval tv;
				gettimeofday(&tv, NULL);
				it->second.last = tv.tv_sec * 1000 + tv.tv_usec / 1000;
			}
			break;
		}
		case 104://控制端想要发起控制（建立udp穿透）【告诉两个客户端你们可以相互连接了】
		{
			ConnectIds ids;
			memcpy(&ids, pack.sData.c_str(), sizeof(ConnectIds));

			std::map<long long, MUserInfo>::iterator it0 = m_mapAddrs.find(ids.id0);
			std::map<long long, MUserInfo>::iterator it1 = m_mapAddrs.find(ids.id1);
			ssize_t ret = 0;
			if ((it0 != m_mapAddrs.end()) && (it1 != m_mapAddrs.end()))
			{
				MUserInfo mInfo0(it1->second.ip, it1->second.port);
				MUserInfo mInfo1(it0->second.ip, it0->second.port);
				CPacket sendPack0(105, (unsigned char*)&mInfo0, sizeof(MUserInfo));
				CPacket sendPack1(105, (unsigned char*)&mInfo1, sizeof(MUserInfo));
				

				ret = send(it0->second.tcpSock, sendPack0.Data(), sendPack0.Size(), 0);
				if (ret <= 0)
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
				ret = send(it1->second.tcpSock, sendPack1.Data(), sendPack1.Size(), 0);
				if (ret <= 0)
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
			}
			else
			{
				CPacket sendPack(106);
				ret = send(m_tcpSock, sendPack.Data(), sendPack.Size(), 0);
				if (ret <= 0)
				{
					printf("%s(%d):%s hu xiang lian jie error (%d) %s\n", __FILE__, __LINE__, __FUNCTION__, errno, strerror(errno));
					break;
				}
			}
			break;
		}
	}

	return 0;
}

int UDPPassNetWork::SendAddrs()
{
	
	//给每个人发
	printf("devices:%lld\n", m_mapAddrs.size());
	for (std::map<long long, MUserInfo>::iterator it = m_mapAddrs.begin(); it != m_mapAddrs.end(); it++)
	{
		if (m_mapAddrs.size() > 1)
		{
			CPacket pack = GetSendAddr(it->first);
			send(it->second.tcpSock, pack.Data(), pack.Size(), 0);
		}
		else
		{
			CPacket pack(102);
			send(it->second.tcpSock, pack.Data(), pack.Size(), 0);
		}
		
	}
	return 0;
}

int UDPPassNetWork::TestOnline()
{
	while (!m_stop)
	{
		if (m_mapAddrs.size() > 0)
		{
			bool isUpdate = false;
			for (std::map<long long, MUserInfo>::iterator it = m_mapAddrs.begin(); it != m_mapAddrs.end(); it++)
			{
				//获得当前时间错（毫秒级别）
				struct timeval tv;
				gettimeofday(&tv, NULL);
				long long tick = tv.tv_sec * 1000 + tv.tv_usec / 1000;
				//和上一次交互的时间大于5秒钟
				if (tick - it->second.last > (5000))
				{
					m_mapAddrs.erase(it);
					isUpdate = true;
				}
			}
			if (isUpdate)
			{
				SendAddrs();
			}
		}
		usleep(1000 * 1000 * 3);
	}
	return 0;
}

CPacket UDPPassNetWork::GetSendAddr(long long id)
{
	std::map<long long, MUserInfo>::iterator find = m_mapAddrs.find(id);
	if (find != m_mapAddrs.end())
	{
		int pos = 0;
		std::unique_ptr<char> data(new char[m_mapAddrs.size() * sizeof(MUserInfo)]);
		memcpy(data.get() + (pos * sizeof(MUserInfo)), &find->second, sizeof(MUserInfo));
		pos++;
		for (std::map<long long, MUserInfo>::iterator it = m_mapAddrs.begin(); it != m_mapAddrs.end(); it++)
		{
			if (it != find)
			{
				memcpy(data.get() + (pos * sizeof(MUserInfo)), &it->second, sizeof(MUserInfo));
				pos++;
			}			
		}
		return CPacket(102, (unsigned char*)data.get(), (uint32_t)(m_mapAddrs.size() * sizeof(MUserInfo)));
	}
	return CPacket(-1);
}

void UDPPassNetWork::EraseAddrBySocket(int sock)
{
	m_mutex.lock();
	for (std::map<long long, MUserInfo>::iterator it = m_mapAddrs.begin(); it != m_mapAddrs.end(); it++)
	{
		if (it->second.tcpSock == sock)
		{
			m_mapAddrs.erase(it);
			break;
		}
	}
	m_mutex.unlock();
}
