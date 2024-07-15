#pragma once
#pragma warning(disable:4267)
#pragma warning(disable:4996)
#include "resource.h"
#include "ServerSocket.h"
#include "LockMachineDlg.h"
#include <io.h>
#include <atlimage.h>
#include <list>
#include <map>

#define WM_LOCKMACHINE		(WM_USER + 1)
#define WM_UNLOCKMACHINE	(WM_USER + 2)

class CCmdProcessor;
typedef void (CCmdProcessor::*CMD_FUNC) (CPacket& recvPack, std::list<CPacket>& sendPacks);

class CCmdProcessor
{
private:
	//CServerSocket* pServer;
	HANDLE						m_hThreadLock;
	UINT						m_nThreadIdLock;
	HANDLE						m_hEventLock;
	std::map<int, CMD_FUNC>		m_mapFuncs;
public:
	CCmdProcessor()
	{
		//pServer = CServerSocket::GetInstance();
		m_hThreadLock = INVALID_HANDLE_VALUE;

		//建立消息映射机制
		struct
		{
			int			nCmd;
			CMD_FUNC	func;
		}
		func_map_table[]
		{
			{1	,&CCmdProcessor::GetDriveInfo	},
			{2	,&CCmdProcessor::GetFileInfo	},
			{3	,&CCmdProcessor::DownLoadFile	},
			{4	,&CCmdProcessor::DelFile		},
			{5	,&CCmdProcessor::ScreenWatch	},
			{6	,&CCmdProcessor::ControlMouse	},
			{7	,&CCmdProcessor::LockMachine	},
			{8	,&CCmdProcessor::UnLockMachine	},
			{-1	,NULL							},
		};

		for (int i = 0; func_map_table[i].func != NULL; i++)
		{
			m_mapFuncs.insert(std::pair<int, CMD_FUNC>
				(func_map_table[i].nCmd,
					func_map_table[i].func));
		}
	}
public:
	void Run()
	{
		
	}

	void DispatchCommand(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		std::map<int, CMD_FUNC>::iterator it = m_mapFuncs.find(recvPack.nCmd);
		if (it != m_mapFuncs.end())
		{
			(this->*(it->second))(recvPack, sendPacks);
		}
	}


private:
	void GetDriveInfo(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		DRIVEINFO driveInfo;
		for (int i = 1; i <= 26; i++)
		{
			if (_chdrive(i) == 0)
			{
				driveInfo.drive[driveInfo.drive_count++] = 'A' + i - 1;
			}
		}
		sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&driveInfo, sizeof(DRIVEINFO)));
	}
	void GetFileInfo(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		std::string path = recvPack.sData;
		if (_chdir(path.c_str()) == 0)
		{
			FILEINFO fileInfo{};
			intptr_t first = _findfirst("*", &fileInfo.data);
			if (first == -1)
			{
				//第一个就没找到：当前这个就是空的
				fileInfo.isNull = 1;
				sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO)));
				return;
			}
			else
			{
				do
				{
					//正在发送
					fileInfo.isNull = 0;
					sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO)));
					memset(&fileInfo, 0, sizeof(FILEINFO));

				} while (_findnext(first, &fileInfo.data) == 0);
				//发完了：当前这个就是空的
				fileInfo.isNull = 1;
				sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO)));
			}
		}
		else
		{
			FILEINFO fileInfo{};
			fileInfo.isNull = 1;
			sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO)));
		}
	}
	void DownLoadFile(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		static const int buffer_size = 1024 * 10;
		static char buffer[buffer_size]{};
		std::string path = recvPack.sData;
		long long fileLen = 0;
		FILE* pFile = fopen(path.c_str(), "rb+");
		if (pFile == NULL)
		{
			//把长度发给控制端，0表示文件为空，或者没有权限
			sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileLen, 8));
			return;
		}
		//获得文件长度
		_fseeki64(pFile, 0, SEEK_END);
		fileLen = _ftelli64(pFile);
		_fseeki64(pFile, 0, SEEK_SET);
		//把长度发给控制端
		sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&fileLen, 8));
		//读取一点发一点
		int readLen = 0;
		while ((readLen = fread(buffer, 1, buffer_size, pFile)) > 0)
		{
			sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)buffer, readLen));
			memset(buffer, 0, buffer_size);
		}
		//发完关闭
		fclose(pFile);
	}
	void DelFile(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		std::string path = recvPack.sData;
		WCHAR widePath[MAX_PATH]{};
		int transRet = MultiByteToWideChar(CP_ACP, 0, path.c_str(), path.size(), widePath, MAX_PATH);
		int success = 0;
		if (DeleteFileW(widePath))
		{
			success = 1;
		}
		else
		{
			success = 0;
		}
		sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)&success, sizeof(int)));
	}
	void ScreenWatch(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{

		CImage screen;
		HDC hScreen = ::GetDC(NULL);
		int nBitperPixel = GetDeviceCaps(hScreen, BITSPIXEL);
		int nWidth = GetDeviceCaps(hScreen, HORZRES);
		int nHeight = GetDeviceCaps(hScreen, VERTRES);
		screen.Create(nWidth, nHeight, nBitperPixel);
		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);

		ReleaseDC(NULL, hScreen);

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) return;
		IStream* pStream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);

		if (ret == S_OK)
		{
			screen.Save(pStream, Gdiplus::ImageFormatPNG);
			LARGE_INTEGER li = { 0 };
			pStream->Seek(li, STREAM_SEEK_SET, NULL);
			LPVOID pData = GlobalLock(hMem);
			sendPacks.push_back(CPacket(recvPack.nCmd, (BYTE*)pData, GlobalSize(hMem), false));

			GlobalUnlock(hMem);
		}
		GlobalFree(hMem);
		screen.ReleaseDC();
		screen.Destroy();
	}
	void ControlMouse(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		//控制端发来的鼠标信息
		MOUSEINFO mouseInfo;
		memcpy(&mouseInfo, recvPack.sData.c_str(), sizeof(MOUSEINFO));
		//组成flags
		int mouseFlags = 0;
		mouseFlags |= mouseInfo.nButton;
		mouseFlags |= mouseInfo.nEvent;
		if (mouseInfo.nButton != MOUSEBTN::NOTHING)
		{
			SetCursorPos(mouseInfo.ptXY.x, mouseInfo.ptXY.y);
		}
		//处理对应事件
		switch (mouseFlags)
		{
			//左键事件处理--------------------------------------------
		case MOUSEBTN::LEFT | MOUSEEVE::CLICK:		//左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::LEFT | MOUSEEVE::DBCLICK:	//左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::LEFT | MOUSEEVE::DOWN:		//左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::LEFT | MOUSEEVE::UP:			//左键弹起
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
			//中键事件处理--------------------------------------------
		case MOUSEBTN::MID | MOUSEEVE::CLICK:			//中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::MID | MOUSEEVE::DBCLICK:			//中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::MID | MOUSEEVE::DOWN:			//中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::MID | MOUSEEVE::UP:				//中键弹起
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
			//右键事件处理--------------------------------------------
		case MOUSEBTN::RIGHT | MOUSEEVE::CLICK:			//右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::RIGHT | MOUSEEVE::DBCLICK:		//右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::RIGHT | MOUSEEVE::DOWN:			//右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case MOUSEBTN::RIGHT | MOUSEEVE::UP:			//右键弹起
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
			//移动事件处理--------------------------------------------
		case MOUSEBTN::NOTHING | MOUSEEVE::MOVE:		//直接移动
			SetCursorPos(mouseInfo.ptXY.x, mouseInfo.ptXY.y);
			break;
		}
		//给个回应
		sendPacks.push_back(CPacket(recvPack.nCmd));
	}
	void LockMachine(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		if (m_hThreadLock == INVALID_HANDLE_VALUE)
		{
			m_hThreadLock = (HANDLE)_beginthreadex(NULL, 0, &ThreadEntryLock, this, 0, &m_nThreadIdLock);
			m_hEventLock = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (m_hEventLock)
			{
				WaitForSingleObject(m_hEventLock, 100);
			}
		}
		int postRet = PostThreadMessage(m_nThreadIdLock, WM_LOCKMACHINE, NULL, NULL);
		if (postRet == 0)
		{
			Sleep(10);
			PostThreadMessage(m_nThreadIdLock, WM_LOCKMACHINE, NULL, NULL);
		}
		//给个回应
		sendPacks.push_back(CPacket(recvPack.nCmd));
	}
	void UnLockMachine(CPacket& recvPack, std::list<CPacket>& sendPacks)
	{
		PostThreadMessage(m_nThreadIdLock, WM_UNLOCKMACHINE, NULL, NULL);
		//给个回应
		sendPacks.push_back(CPacket(recvPack.nCmd));
	}
	static unsigned __stdcall ThreadEntryLock(void* arg)
	{
		CCmdProcessor* thiz = (CCmdProcessor*)arg;
		thiz->ThreadLock();
		_endthreadex(0);
		return 0;
	}
	void ThreadLock()
	{
		SetEvent(m_hEventLock);
		CLockMachineDlg lockDlg;
		lockDlg.Create(IDD_LOCKMACHINEDLG);
		::SetWindowPos(lockDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		CRect screenRect(0, 0, screenWidth, (int)(screenHeight * 1.1));
		lockDlg.MoveWindow(screenRect);
		CRect txtInfoRectOld;
		lockDlg.m_txtInfo.GetWindowRect(&txtInfoRectOld);
		CRect txtInfoRectNew;
		txtInfoRectNew.left = (int)(screenRect.Width() / 2.0 - txtInfoRectOld.Width() / 2);
		txtInfoRectNew.top = (int)(screenRect.Height() / 2.0 - txtInfoRectOld.Height() / 2);
		txtInfoRectNew.right = txtInfoRectNew.left + txtInfoRectOld.Width();
		txtInfoRectNew.bottom = txtInfoRectNew.top + txtInfoRectOld.Height();
		lockDlg.m_txtInfo.MoveWindow(txtInfoRectNew);

		MSG msg;
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_LOCKMACHINE)
			{
				lockDlg.ShowWindow(SW_SHOW);
				lockDlg.CenterWindow();
			}

			if (msg.message == WM_UNLOCKMACHINE)
			{
				lockDlg.ShowWindow(SW_HIDE);
			}
		}

	}
};
//
//	void GetDriveInfo(WORD _nCmd)
//	{
//		DRIVEINFO driveInfo;
//		for (int i = 1; i <= 26; i++)
//		{
//			if (_chdrive(i) == 0)
//			{
//				driveInfo.drive[driveInfo.drive_count++] = 'A' + i - 1;
//			}
//		}
//		CPacket pack(_nCmd, (BYTE*)&driveInfo, sizeof(DRIVEINFO));
//		pServer->Send(pack);
//		pServer->CloseSocket();
//	}
//	void GetFileInfo(WORD _nCmd)
//	{
//		std::string path;
//		pServer->GetPath(path);
//		if (_chdir(path.c_str()) == 0)
//		{
//			FILEINFO fileInfo{};
//			intptr_t first = _findfirst("*", &fileInfo.data);
//			if (first == -1)
//			{
//				//第一个就没找到：当前这个就是空的
//				fileInfo.isNull = 1;
//				CPacket pack(_nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO));
//				pServer->Send(pack);
//				pServer->CloseSocket();
//
//				return;
//			}
//			else
//			{
//				do
//				{
//					//正在发送
//					fileInfo.isNull = 0;
//					CPacket pack(_nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO));
//					int ret = pServer->Send(pack);
//					TRACE("-server-nCmd = %d name = %s\r\n", ret, fileInfo.data.name);
//					memset(&fileInfo, 0, sizeof(FILEINFO));
//
//				} while (_findnext(first, &fileInfo.data) == 0);
//				//发完了：当前这个就是空的
//				fileInfo.isNull = 1;
//				CPacket pack(_nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO));
//				pServer->Send(pack);
//				pServer->CloseSocket();
//
//			}
//		}
//		else
//		{
//			FILEINFO fileInfo{};
//			fileInfo.isNull = 1;
//			CPacket pack(_nCmd, (BYTE*)&fileInfo, sizeof(FILEINFO));
//			pServer->Send(pack);
//			pServer->CloseSocket();
//		}
//	}
//	void DownLoadFile(WORD _nCmd)
//	{
//		static const int buffer_size = 1024 * 10;
//		static char buffer[buffer_size]{};
//		std::string path;
//		pServer->GetPath(path);
//		long long fileLen = 0;
//		FILE* pFile = fopen(path.c_str(), "rb+");
//		if (pFile == NULL)
//		{
//			//把长度发给控制端，0表示文件为空，或者没有权限
//			CPacket pack(_nCmd, (BYTE*)&fileLen, 8);
//			pServer->Send(pack);
//			pServer->CloseSocket();
//			return;
//		}
//		//获得文件长度
//		_fseeki64(pFile, 0, SEEK_END);
//		 fileLen = _ftelli64(pFile);
//		_fseeki64(pFile, 0, SEEK_SET);
//		//把长度发给控制端
//		CPacket pack(_nCmd, (BYTE*)&fileLen, 8);
//		pServer->Send(pack);
//		//读取一点发一点
//		int readLen = 0;
//		while ((readLen = fread(buffer, 1, buffer_size, pFile)) > 0)
//		{
//			CPacket pack(_nCmd, (BYTE*)buffer, readLen);
//			pServer->Send(pack);
//			memset(buffer, 0, buffer_size);
//		}
//		//发完关闭
//		pServer->CloseSocket();
//		fclose(pFile);
//	}
//	void DelFile(WORD _nCmd)
//	{
//		std::string path;
//		pServer->GetPath(path);
//		WCHAR widePath[MAX_PATH]{};
//		int transRet = MultiByteToWideChar(CP_ACP, 0, path.c_str(), path.size(), widePath, MAX_PATH);
//		int success = 0;
//		if (DeleteFileW(widePath))
//		{
//			success = 1;		
//		}
//		else
//		{
//			success = 0;	
//		}
//		CPacket pack(_nCmd, (BYTE*)&success, sizeof(int));
//		pServer->Send(pack);
//		pServer->CloseSocket();
//	}
//	void ScreenWatch(WORD _nCmd)
//	{
//		//屏幕截图
//		/*CImage screen;
//		HDC hScreen = ::GetDC(NULL);
//		int nBitperPixel = GetDeviceCaps(hScreen, BITSPIXEL);
//		int nWidth = GetDeviceCaps(hScreen, HORZRES);
//		int nHeight = GetDeviceCaps(hScreen, VERTRES);
//		screen.Create(nWidth, nHeight, nBitperPixel);
//		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
//		ReleaseDC(NULL, hScreen);
//
//		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
//		if (hMem == NULL) return -1;
//		IStream* pStream = NULL;
//		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
//
//		if (ret == S_OK)
//		{
//			screen.Save(pStream, Gdiplus::ImageFormatPNG);
//		}*/
//
//		CImage screen;
//		HDC hScreen = ::GetDC(NULL);
//		int nBitperPixel = GetDeviceCaps(hScreen, BITSPIXEL);
//		int nWidth = GetDeviceCaps(hScreen, HORZRES);
//		int nHeight = GetDeviceCaps(hScreen, VERTRES);
//		screen.Create(nWidth, nHeight, nBitperPixel);
//		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);
//		ReleaseDC(NULL, hScreen);
//
//		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
//		if (hMem == NULL) return;
//		IStream* pStream = NULL;
//		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
//
//		if (ret == S_OK)
//		{
//			screen.Save(pStream, Gdiplus::ImageFormatPNG);
//			LARGE_INTEGER li = { 0 };
//			pStream->Seek(li, STREAM_SEEK_SET, NULL);
//			LPVOID pData = GlobalLock(hMem);
//			CPacket pack(_nCmd, (BYTE*)pData, GlobalSize(hMem));
//			pServer->Send(pack);
//			GlobalUnlock(hMem);
//		}
//		GlobalFree(hMem);
//		screen.ReleaseDC();
//		screen.Destroy();
//	}
//	void ControlMouse(WORD _nCmd)
//	{
//		//控制端发来的鼠标信息
//		MOUSEINFO mouseInfo;
//		memcpy(&mouseInfo, pServer->GetPacket().sData.c_str(), sizeof(MOUSEINFO));
//		//组成flags
//		int mouseFlags = 0;
//		mouseFlags |= mouseInfo.nButton;
//		mouseFlags |= mouseInfo.nEvent;
//		if (mouseInfo.nButton != MOUSEBTN::NOTHING)
//		{
//			SetCursorPos(mouseInfo.ptXY.x, mouseInfo.ptXY.y);
//		}
//		//处理对应事件
//		switch (mouseFlags)
//		{
//		//左键事件处理--------------------------------------------
//		case MOUSEBTN::LEFT		| MOUSEEVE::CLICK:		//左键单击
//			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::LEFT		| MOUSEEVE::DBCLICK:	//左键双击
//			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::LEFT		| MOUSEEVE::DOWN:		//左键按下
//			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::LEFT		| MOUSEEVE::UP:			//左键弹起
//			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		//中键事件处理--------------------------------------------
//		case MOUSEBTN::MID | MOUSEEVE::CLICK:			//中键单击
//			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::MID | MOUSEEVE::DBCLICK:			//中键双击
//			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::MID | MOUSEEVE::DOWN:			//中键按下
//			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::MID | MOUSEEVE::UP:				//中键弹起
//			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		//右键事件处理--------------------------------------------
//		case MOUSEBTN::RIGHT | MOUSEEVE::CLICK:			//右键单击
//			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::RIGHT | MOUSEEVE::DBCLICK:		//右键双击
//			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::RIGHT | MOUSEEVE::DOWN:			//右键按下
//			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		case MOUSEBTN::RIGHT | MOUSEEVE::UP:			//右键弹起
//			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
//			break;
//		//移动事件处理--------------------------------------------
//		case MOUSEBTN::NOTHING	| MOUSEEVE::MOVE:		//直接移动
//			SetCursorPos(mouseInfo.ptXY.x, mouseInfo.ptXY.y);
//			break;
//		}
//		//给个回应
//		CPacket pack(_nCmd);
//		pServer->Send(pack);
//	}
//	void LockMachine(WORD _nCmd)
//	{
//		if (m_hThreadLock == INVALID_HANDLE_VALUE)
//		{
//			m_hThreadLock = (HANDLE)_beginthreadex(NULL,0,&ThreadEntryLock,this, 0, &m_nThreadIdLock);
//			m_hEventLock = CreateEvent(NULL, TRUE, FALSE, NULL);
//			if (m_hEventLock)
//			{
//				WaitForSingleObject(m_hEventLock, 100);
//			}
//		}
//		int postRet = PostThreadMessage(m_nThreadIdLock, WM_LOCKMACHINE,NULL,NULL);
//		if (postRet == 0)
//		{
//			Sleep(10);
//			PostThreadMessage(m_nThreadIdLock, WM_LOCKMACHINE, NULL, NULL);
//		}
//		//给个回应
//		CPacket pack(_nCmd);
//		pServer->Send(pack);
//	}
//	void UnLockMachine(WORD _nCmd)
//	{
//		PostThreadMessage(m_nThreadIdLock, WM_UNLOCKMACHINE, NULL, NULL);
//		//给个回应
//		CPacket pack(_nCmd);
//		pServer->Send(pack);
//	}
//	static unsigned __stdcall ThreadEntryLock(void* arg)
//	{
//		CCmdProcessor* thiz = (CCmdProcessor*)arg;
//		thiz->ThreadLock();
//		_endthreadex(0);
//		return 0;
//	}
//	void ThreadLock()
//	{
//		SetEvent(m_hEventLock);	
//		CLockMachineDlg lockDlg;
//		lockDlg.Create(IDD_LOCKMACHINEDLG);
//		::SetWindowPos(lockDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
//		int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
//		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
//		CRect screenRect(0,0,screenWidth,(int)(screenHeight * 1.1));
//		lockDlg.MoveWindow(screenRect);
//		CRect txtInfoRectOld;
//		lockDlg.m_txtInfo.GetWindowRect(&txtInfoRectOld);
//		CRect txtInfoRectNew;
//		txtInfoRectNew.left   = (int)(screenRect.Width()  / 2.0 - txtInfoRectOld.Width()  / 2);
//		txtInfoRectNew.top    = (int)(screenRect.Height() / 2.0 - txtInfoRectOld.Height() / 2);
//		txtInfoRectNew.right  = txtInfoRectNew .left + txtInfoRectOld.Width();
//		txtInfoRectNew.bottom = txtInfoRectNew.top  + txtInfoRectOld.Height();
//		lockDlg.m_txtInfo.MoveWindow(txtInfoRectNew);
//
//		MSG msg;
//		while (::GetMessage(&msg,NULL,0,0))
//		{
//			TranslateMessage(&msg);
//			DispatchMessage (&msg);
//
//			if (msg.message == WM_LOCKMACHINE)
//			{
//				lockDlg.ShowWindow(SW_SHOW);
//				lockDlg.CenterWindow();
//			}
//
//			if (msg.message == WM_UNLOCKMACHINE)
//			{
//				lockDlg.ShowWindow(SW_HIDE);
//			}
//		}
//
//	}
//};

