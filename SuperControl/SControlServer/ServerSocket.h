#pragma once
#include "Common.h"
#include <Windows.h>
#include <direct.h>
#include <vector>
class CServerSocket
{
#define BUFFER_SIZE (1024*1024)
//------------------------ʵ�ֵ�������--------------------------//
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
	/// ɾ����������
	/// </summary>
	/// <returns></returns>
	static void DesInstance()
	{
		delete m_instance;
	}
public:
	/// <summary>
	/// ��ȡ��������
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
//------------------------ʵ�ֵ�������--------------------------//


 
//------------------------ʵ��ҵ����--------------------------//
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
public:
	/// <summary>
	/// ��ʼ���׽���
	/// </summary>
	void InitSocket()
	{
		//�����׽���
		serv_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (serv_sock == INVALID_SOCKET)
		{
			TRACE("�����׽���ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"�����׽���ʧ��");
			return;
		}
		//���õ�ַ���˿�
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(6888);
		//�󶨵�ַ���˿�
		if (bind(serv_sock, (SOCKADDR*)&serv_addr, serv_addr_len) == SOCKET_ERROR)
		{
			closesocket(serv_sock);
			TRACE("�󶨶˿ڵ�ַʧ��(������:%d ����:%s)\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()));
			AfxMessageBox(L"�󶨶˿ڵ�ַʧ��");
			return;
		}
		//�����׽���
		if (listen(serv_sock, 3) == SOCKET_ERROR)
		{
			closesocket(serv_sock);
			TRACE("�����׽���ʧ��(������:%d ����:%s)\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()));
			AfxMessageBox(L"�����׽���ʧ��");
			return;
		}
	}
	void CloseSocket()
	{
		closesocket(clnt_sock);
	}
	/// <summary>
	/// ���ܿͻ�����
	/// </summary>
	/// <returns></returns>
	bool AcceptClient()
	{
		//���ܿͻ�����
		clnt_sock = accept(serv_sock, (SOCKADDR*)&clnt_addr, &clnt_addr_len);
		return clnt_sock != INVALID_SOCKET;
	}
	/// <summary>
	/// ����������������
	/// </summary>
	int DealCommand()
	{
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("��������ʱ���ͻ�δ���� (%d)\r\n",clnt_sock);
			return -1;
		}
		int readLen = recv(clnt_sock, buffer.data(), BUFFER_SIZE, 0);
		if (readLen <= 0)
		{
			TRACE("��������ʱ����ȡ���ݴ��� (%d)\r\n",readLen);
			return -2;
		}
		DWORD len = readLen;
		pack = CPacket((BYTE*)buffer.data(), len);
		if (len <= 0)
		{
			TRACE("��������ʱ���������ݰ�ʧ�� (%d)\r\n", len);
			return -3;
		}
		return pack.nCmd;
	}
	/// <summary>
	/// �������ݰ�
	/// </summary>
	/// <param name="_pack">���ݰ�</param>
	int Send(CPacket& _pack)
	{
		//Dump(_pack);
		return send(clnt_sock, (char*)_pack.Data(), _pack.Size(), 0);
	}
	/// <summary>
	/// �õ��ս����İ�
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
//------------------------ʵ��ҵ����--------------------------//
