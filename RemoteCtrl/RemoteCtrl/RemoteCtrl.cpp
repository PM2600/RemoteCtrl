// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "ServerSocket.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <list>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象
CWinApp theApp;

using namespace std;

void Dump(BYTE* pData, size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize; i++) {
        char buf[8] = "";
        if (i > 0 && (i % 16 == 0)) {
            strOut += "\n";
        }
        snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

int MakeDiverInfo() {
    std::string result;
    for (int i = 1; i < 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0) {
                result += ',';
            }
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());
    //CServerSocket::getInstance()->Send(pack);
    return 0;
}

typedef struct file_info{
    file_info() {
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid; // 是否有效
    BOOL IsDirectory; // 0目录，1文件
    BOOL HasNext; // 是否还有后续 0没有，1有
    char szFileName[256]; // 文件名 
}FILEINFO, *PFILEINFO;

int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO> listFileInfos;
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前命令不是获取文件列表，命令解析错误"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        CPacket pack(2, (BYTE*)& finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //listFileInfos.push_back(finfo);
        OutputDebugString(_T("没有权限访问目录"));
        return -2;
    }
    _finddata_t fdata;
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件"));
        return -3;
    }

    do {
        FILEINFO finfo;
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        //listFileInfos.push_back(finfo);
    } while (!_findnext(hfind, &fdata));
    
    FILEINFO finfo;
    finfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket pack(3, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}
#pragma warning(disable:4996)

int DownLoadFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile, strPath.c_str(), "rb");
    
    if (err != 0) {
        CPacket pack(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024] = "";
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);
        fclose(pFile);
    }
    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int MouseEvent() {
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
        DWORD nFlags = 0;
        switch (mouse.nButton) {
        case 0: // 左键
            nFlags = 1;
            break;
        case 1: // 右键
            nFlags = 2;
            break;
        case 2: // 中键
            nFlags = 4;
            break;
        case 3: // 没有按键
            nFlags = 8;
            break;
        }
        if (nFlags != 8) {
            SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
        }

        switch (mouse.nAction) {
        case 0: // 单击
            nFlags |= 0x10;
            break;
        case 1: // 双击
            nFlags |= 0x20;
            break;
        case 2: // 按下
            nFlags |= 0x40;
            break;
        case 3: // 放开
            nFlags |= 0x80;
            break;
        default:
            break;
        }

        switch (nFlags) {
        case 0x21://左键双击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单击
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://右键单击
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x24://中键双击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://中键单击
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中键放开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());

            break;
        case 0x08://鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }

        CPacket pack(5, NULL, 0);
        CServerSocket::getInstance()->Send(pack);
    }
    else {
        OutputDebugString(_T("获取鼠标参数失败"));
        return -1;
    }
    return 0;
}

int SendScreen() {
    CImage screen;
    HDC hScreen = ::GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
    int nWidth = GetDeviceCaps(hScreen, HORZRES);
    int nHeight = GetDeviceCaps(hScreen, VERTRES);
    screen.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(screen.GetDC(), 0, 0, 1920, 1030, hScreen, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hScreen);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
    if (hMem == NULL) {
        return -1;
    }

    IStream* pStream = NULL;
    HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
    if (ret == S_OK) {
        screen.Save(pStream, Gdiplus::ImageFormatPNG);
        LARGE_INTEGER bg = { 0 };
        pStream->Seek(bg, STREAM_SEEK_SET, NULL);
        PBYTE pData = (PBYTE)GlobalLock(hMem);
        SIZE_T nSize = GlobalSize(hMem);
        CPacket pack(6, pData, nSize);
        CServerSocket::getInstance()->Send(pack);
        GlobalUnlock(hMem);   
    }

    //DWORD tick = GetTickCount64();
    //screen.Save(_T("test2024.png"), Gdiplus::ImageFormatPNG);
    //TRACE("png: %d\r\n", GetTickCount64() - tick);
    //DWORD tick = GetTickCount64();
    //screen.Save(_T("test2024.jpg"), Gdiplus::ImageFormatJPEG);
    //TRACE("jpg: %d\r\n", GetTickCount64() - tick);

    pStream->Release();
    GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}

#include "LockInfoDialog.h"
CLockInfoDialog dlg;
unsigned threadid = 0;

unsigned __stdcall threadLockDlg(void* arg) {
    dlg.Create(IDD_DIALOG_INFO, NULL);
    dlg.ShowWindow(SW_SHOW);
    // 遮蔽后台窗口    
    CRect rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    rect.bottom *= 1.025;
    dlg.MoveWindow(rect);
    // 窗口置顶
    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    // 限制鼠标功能
    ShowCursor(false);
    // 隐藏任务栏
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);
    // dlg.GetWindowRect(rect);
    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    // 限制鼠标活动范围
    ClipCursor(rect);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == 0x41) // 按下esc来退出
                break;
        }
    }
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
    ShowCursor(true);
    dlg.DestroyWindow();
    _endthreadex(0);
    return 0;
}

int LockMachine() {
    if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);
    }
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int UnlockMachine() {
    PostThreadMessage(threadid, WM_KEYDOWN, 0x41, 0);
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect() {
    //TRACE("creat 1981pack\r\n");
    CPacket pack(1981, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int ExcuteCommand(int nCmd) {
    int ret = 0;
    switch (nCmd) {
    case1: // 查看磁盘分区
        ret = MakeDiverInfo();
        break;
    case 2: // 查看指定目录下的文件
        ret = MakeDirectoryInfo();
        break;
    case 3: // 打开文件
        ret = RunFile();
        break;
    case 4: // 下载文件
        ret = DownLoadFile();
        break;
    case 5: // 鼠标操作
        ret = MouseEvent();
        break;
    case 6: // 发送屏幕内容 --> 发送屏幕的截图
        ret = SendScreen();
        break;
    case 7: // 锁机
        ret = LockMachine();
        break;
    case 8: // 解锁
        ret = UnlockMachine();
        break;
    case 1981:
        ret = TestConnect();
        break;
    }
    return ret;
}

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
            // TODO: 在此处为应用程序的行为编写代码。
            CServerSocket* pserver = CServerSocket::getInstance();
            int count = 0;
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T(""), _T("网络初始化异常，请检查网络状态"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while ((pserver = (CServerSocket::getInstance())) != NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("多次无法接入用户， 结束程序"), _T("失败，"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("自动重试"), _T("接入用户失败，"),  MB_OK | MB_ICONERROR);
                    count++;
                }
                TRACE("Accept return true\r\n");
                int ret = pserver->DealCommand();
                TRACE("DealCommand ret=%d\r\n", ret);
                if (ret > 0) {
                    ret = ExcuteCommand(ret);
                    if (ret != 0) {
                        TRACE("执行命令失败: %d ret=%d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();
                }    
            }
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
