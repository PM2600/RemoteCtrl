// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include "Tool.h"
#include "Command.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define INVOKE_PATH _T("C:\\Users\\pm26\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe")
//#define INVOKE_PATH _T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe")

// 唯一的应用程序对象
CWinApp theApp;

using namespace std;

bool ChooseAutoInvoke(const CString& strPath) {
    TCHAR wcsSystem[MAX_PATH] = _T("");
    //CString strPath = CString(_T("C:\\Windows\\SysWOW64\\RemoteCtrl.exe"));
    //CString strPath = _T("C:\\Users\\PM2.5\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\RemoteCtrl.exe");
    if (PathFileExists(strPath)) {
        return;
    }
    CString strInfo = _T("该程序只允许用于合法的用途！\n");
    strInfo += _T("继续运行该程序，将使得这台机器处于被监控状态！\n");
    strInfo += _T("如果你不希望这样，请按“取消”按钮，退出程序！\n");
    strInfo += _T("按下“是”按钮，该程序将被复制到你的机器上，并随系统启动而自动运行！\n");
    strInfo += _T("按下“否”按钮，程序只会运行一次，不会在系统内留下任何东西！\n");
    int ret = MessageBox(NULL, strInfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
    if (ret == IDYES) {
        //if (CTool::WriteRegisterTable(strPath)) {
        //    MessageBox(NULL, _T("复制文件失败，是否权限不足\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
        //    return false;
        //}
        if (!CTool::WriteStartupDir(strPath)) {
            MessageBox(NULL, _T("复制文件失败，是否权限不足\r\n"), _T("错误"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
    }
    else if (ret == IDCANCEL) {
        return false;
    }
    return true;
}

HANDLE hIOCP = INVALID_HANDLE_VALUE; // IO Completion Port

int main()
{
    if (!CTool::Init())
        return 1;
    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);

    

    //if (CTool::IsAdmin()) {
    //    if (!CTool::Init())
    //        return 1;
    //    MessageBox(NULL, _T("管理员"), _T("用户状态"), 0);
    //    if (ChooseAutoInvoke(INVOKE_PATH)) {
    //        CCommand cmd;
    //        int ret = CServerSocket::getInstance()->Run(&CCommand::RunCommand, &cmd);
    //        switch (ret) {
    //        case -1:
    //            MessageBox(NULL, _T(""), _T("网络初始化异常，请检查网络状态"), MB_OK | MB_ICONERROR);
    //            break;
    //        case -2:
    //            MessageBox(NULL, _T("多次无法接入用户， 结束程序"), _T("失败，"), MB_OK | MB_ICONERROR);
    //            break;
    //        }
    //    }
    //}
    //else {
    //    MessageBox(NULL, _T("普通用户"), _T("用户状态"), 0);
    //    if (CTool::RunAsAdmin() == false) {
    //        CTool::ShowError();
    //        return 1;
    //    }
    //}
    return 0;
}