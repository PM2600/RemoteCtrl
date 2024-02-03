#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>
#include "Resource.h"
#include "Tool.h"

#define WM_SEND_PACK (WM_USER + 1)   // 发送包数据
#define WM_SEND_DATA (WM_USER + 2)   // 发送数据
#define WM_SHOW_STATUS (WM_USER + 3) // 展示状态
#define WM_SHOW_WATCH (WM_USER + 4)  // 远程监控
#define WM_SEND_MESSAGE (WM_USER + 0x1000)  // 自定义消息处理


class CClientController
{
public:
	static CClientController* getInstance(); // 获取全局唯一对象
	int InitController();
	int Invoke(CWnd*& pMainWnd);
	LRESULT SendMessage(MSG msg);
	void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpdateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		CClientSocket::getInstance()->CloseSocket();
	}
	bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSocket() == false) {
			return false;
		}
		pClient->Send(pack);
	}
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSocket() == false) {
			return false;
		}
		pClient->Send(CPacket(nCmd, pData, nLength));
		int cmd = DealCommand();
		TRACE("ack: %d\r\n", cmd);
		if (bAutoClose)
			CloseSocket();
		return cmd;
	}

	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CTool::Byte2Image(image, pClient->GetPacket().strData);	
	}

protected:
	CClientController():m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg){
		m_hThread = INVALID_HANDLE_VALUE;
		m_nThreadID = -1;
	}
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned __stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != NULL) {
			delete m_instance;
			m_instance = NULL;
		}
	}

	LRESULT OnSendPack(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam);



private:
	typedef struct MsgInfo{
		MSG msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			memcpy(&msg, &m, sizeof(MSG));
		}

		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				memcpy(&msg, &m.msg, sizeof(MSG));
			}
			return *this;
		}
	}MSGINFO;

	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	CWatchDialog m_watchDlg;
	CRemoteClientDlg m_remoteDlg;
	CStatusDlg m_statusDlg;
	HANDLE m_hThread;
	unsigned m_nThreadID;

	static CClientController* m_instance;

	class CHelper {
	public:
		CHelper() {
			CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

