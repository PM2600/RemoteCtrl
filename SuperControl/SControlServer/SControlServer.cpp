// SControlServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "SControlServer.h"
#include "CmdProcessor.h"
#include "Tool.h"
#include "IocpServer.h"
#include "MQueue.h"
#include "UDPPassServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象
CWinApp theApp;

using namespace std;

int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			//开机自启动
			/*if ((CTool::AutoInvokeS(_T("super-control.exe")) == -1) && (CTool::AutoInvokeX(_T("super-control.exe")) == -1))
			{
				exit(0);
			}*/

			CIocpServer server;
			server.StartServer();


			int _ = getchar();
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}

	return nRetCode;
}
