#pragma once
#include "Common.h"
#include <Windows.h>
#include <direct.h>
#include <vector>
class CServerSocket
{
#define BUFFER_SIZE (1024*1024)
//------------------------实现单例功能--------------------------//
private:
	class CLifeManager
	{
	public:
		 CLifeManager() { GetInstance(); }
		~CLifeManager() { DesInstance(); }
	};
private:
	 CServerSocket() : serv_sock(INVALID_SOCKET) , clnt_sock(INVALID_SOCKET) , serv_addr() , clnt_addr() , serv_addr_len(sizeof(SOCKADDR_IN)) , clnt_addr_len(sizeof(SOCKADDR_IN))
	 {
		 InitSockEnv();
		 buffer.resize(BUFFER_SIZE);
	 }
	~CServerSocket() 
	{
		DesSockEnv();
	}
private:
	static CServerSocket* m_instance;
	static CLifeManager m_life;
private:
	/// <summary>
	/// 删除单例对象
	/// </summary>
	/// <returns></returns>
	static void DesInstance()
	{
		delete m_instance;
	}
public:
	/// <summary>
	/// 获取单例对象
	/// </summary>
	/// <returns></returns>
	static CServerSocket* GetInstance()
	{
		if (m_instance == NULL)
		{
			m_instance = new CServerSocket;
		}
		return m_instance;
	}
//------------------------实现单例功能--------------------------//


 
//------------------------实现业务功能--------------------------//
private:
	SOCKET				serv_sock;
	SOCKET				clnt_sock;
	SOCKADDR_IN			serv_addr;
	SOCKADDR_IN			clnt_addr;
	int					serv_addr_len;
	int					clnt_addr_len;
	std::vector<char>	buffer;
	CPacket				pack;
private:
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
public:
	/// <summary>
	/// 初始化套接字
	/// </summary>
	void InitSocket()
	{
		//创建套接字
		serv_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (serv_sock == INVALID_SOCKET)
		{
			TRACE("创建套接字失败(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"创建套接字失败");
			return;
		}
		//配置地址，端口
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(6888);
		//绑定地址，端口
		if (bind(serv_sock, (SOCKADDR*)&serv_addr, serv_addr_len) == SOCKET_ERROR)
		{
			closesocket(serv_sock);
			TRACE("绑定端口地址失败(错误码:%d 错误:%s)\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()));
			AfxMessageBox(L"绑定端口地址失败");
			return;
		}
		//监听套接字
		if (listen(serv_sock, 3) == SOCKET_ERROR)
		{
			closesocket(serv_sock);
			TRACE("监听套接字失败(错误码:%d 错误:%s)\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()));
			AfxMessageBox(L"监听套接字失败");
			return;
		}
	}
	void CloseSocket()
	{
		closesocket(clnt_sock);
	}
	/// <summary>
	/// 接受客户连接
	/// </summary>
	/// <returns></returns>
	bool AcceptClient()
	{
		//接受客户连接
		clnt_sock = accept(serv_sock, (SOCKADDR*)&clnt_addr, &clnt_addr_len);
		return clnt_sock != INVALID_SOCKET;
	}
	/// <summary>
	/// 解析包，返回命令
	/// </summary>
	int DealCommand()
	{
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("处理命令时，客户未连接 (%d)\r\n",clnt_sock);
			return -1;
		}
		int readLen = recv(clnt_sock, buffer.data(), BUFFER_SIZE, 0);
		if (readLen <= 0)
		{
			TRACE("处理命令时，读取数据错误 (%d)\r\n",readLen);
			return -2;
		}
		DWORD len = readLen;
		pack = CPacket((BYTE*)buffer.data(), len);
		if (len <= 0)
		{
			TRACE("处理命令时，解析数据包失败 (%d)\r\n", len);
			return -3;
		}
		return pack.nCmd;
	}
	/// <summary>
	/// 发送数据包
	/// </summary>
	/// <param name="_pack">数据包</param>
	int Send(CPacket& _pack)
	{
		//Dump(_pack);
		return send(clnt_sock, (char*)_pack.Data(), _pack.Size(), 0);
	}
	/// <summary>
	/// 得到刚解析的包
	/// </summary>
	CPacket& GetPacket()
	{
		return pack;
	}
	void GetPath(std::string& path)
	{
		switch (pack.nCmd)
		{
		case 2:
		case 3:
		case 4:
			path.resize(pack.sData.size());
			memcpy((void*)path.c_str(), pack.sData.c_str(), pack.sData.size());
			break;
		}
	}
};
//------------------------实现业务功能--------------------------//
