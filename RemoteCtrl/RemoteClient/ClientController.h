#pragma once
#include "ClientSocket.h"
#include "WatchDialog.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include <map>

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

protected:
	CClientController() {

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
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC> m_mapFunc;
	std::map<UUID, MSG> m_mapMessage;

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

