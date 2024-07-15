#pragma once
#include "Common.h"
#include <Windows.h>
#include <direct.h>
#include <vector>
class CClientSocket
{

public:

	CClientSocket() : clnt_sock(INVALID_SOCKET), serv_addr(), serv_addr_len(sizeof(SOCKADDR_IN)), isReadOver(false) //, index(0)
	{
		BUFFER_SIZE = 1024 * 1024 * 2;
		InitSockEnv();
		buffer.resize(BUFFER_SIZE);
		memset(buffer.data(), 0, BUFFER_SIZE);
	}
	~CClientSocket()
	{
		DesSockEnv();
	}

	void SetBufferSize(DWORD size)
	{
		BUFFER_SIZE = size;
		buffer.resize(BUFFER_SIZE);
		memset(buffer.data(), 0, BUFFER_SIZE);
	}

private:
	SOCKET				clnt_sock;
	SOCKADDR_IN			serv_addr;
	int					serv_addr_len;
	std::vector<char>	buffer;
	CPacket				pack;
	bool				isReadOver;
	DWORD				BUFFER_SIZE;
	int					index;

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
	void InitSocket(const CString& ip, const CString& port)
	{
		//�����׽���
		clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("�����׽���ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"�����׽���ʧ��");
			return;
		}
		char strPort[256]{};
		if (WideCharToMultiByte(CP_ACP, 0, ip, -1, strPort, 256, NULL, NULL) == 0)
		{
			TRACE("���ֽ�ת���ֽ�ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"���ֽ�ת���ֽ�ʧ��");
			return;
		}
		//���õ�ַ���˿�
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(strPort);
		serv_addr.sin_port = htons(_wtoi(port));
		//���ӱ��ض�
		if (connect(clnt_sock, (SOCKADDR*)&serv_addr, serv_addr_len) == SOCKET_ERROR)
		{
			closesocket(clnt_sock);
			TRACE("���ӷ�����ʧ��(������:%d ����:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"���ӷ�����ʧ��");
			return;
		}
	}

	void CloseSocket()
	{
		isReadOver = false;
		closesocket(clnt_sock);
	}

	/// <summary>
	/// ����������������
	/// </summary>
	int DealCommand()
	{
		//static int index = 0;
		char* buf = buffer.data();
		if (clnt_sock == INVALID_SOCKET)
		{
			TRACE("��������ʱ��δ���� (%d)\r\n", clnt_sock);
			return -1;
		}
		while (true)
		{
			int readLen = 0;	

			TRACE("1------------------tick = %lld\r\n", GetTickCount64());
			readLen = recv(clnt_sock, buf + index, BUFFER_SIZE - index, 0);
			TRACE("2------------------tick = %lld\r\n", GetTickCount64());

			TRACE("[threadId: %d] -------------------------------------------DealCommand----------------------------------------\r\n",GetThreadId(GetCurrentThread()));

			TRACE("[threadId: %d] readLen = %d\r\n", GetThreadId(GetCurrentThread()), readLen);

			if (readLen <= 0 && index <= 0)
			{
				TRACE("[threadId: %d] ��ȡ�������������ˣ����һ����������� \r\n", GetThreadId(GetCurrentThread()));
				return -2;
			}
			TRACE("[threadId: %d] index 1 = %d\r\n", GetThreadId(GetCurrentThread()) , index);
			index += readLen;
			TRACE("[threadId: %d] index 2 = %d\r\n", GetThreadId(GetCurrentThread()), index);

			int len = index;
			TRACE("[threadId: %d] len 1 = %d\r\n", GetThreadId(GetCurrentThread()), len);
			pack = CPacket((BYTE*)buf, len);
			TRACE("[threadId: %d] len 2 = %d\r\n", GetThreadId(GetCurrentThread()), len);

			TRACE("[threadId: %d] pack => %s \r\n", GetThreadId(GetCurrentThread()), pack.ToString2().c_str());

			DWORD thid = GetThreadId(GetCurrentThread());

			if (len > 0)
			{
				TRACE("[threadId: %d] index 3 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				memmove(buf, buf + len, index - len);
				TRACE("[threadId: %d] index 4 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				index -= len;
				TRACE("[threadId: %d] index 5 = %d\r\n", GetThreadId(GetCurrentThread()), index);
				return pack.nCmd;
			}
		}
		TRACE("[threadId: %d] ��ȡ�������� \r\n", GetThreadId(GetCurrentThread()));
		return 0;
	}
	/// <summary>
	/// �������ݰ�
	/// </summary>
	/// <param name="_pack">���ݰ�</param>
	int Send(CPacket& _pack)
	{
		return send(clnt_sock, (char*)_pack.Data(), _pack.Size(), 0);
	}
	/// <summary>
	/// �õ��ս����İ�
	/// </summary>
	CPacket& GetPacket()
	{
		return pack;
	}
};
