#pragma once
class CTool
{
public:
    static void Dump(BYTE* pData, size_t nSize) {
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

    static bool IsAdmin() {
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len) == false) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) {
            return eve.TokenIsElevated;
        }
        //printf("length of token is %s\r\n", len);
        return false;
    }

    static bool RunAsAdmin() {
        // ��ȡ����ԱȨ�ޣ�ʹ�ø�Ȩ�޴����½���
        // ���ز����� ����Administrator�˻� ��ֹ������ֻ�ܵ�¼���ؿ���̨
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CreateProcessWithLogonW(_T("Administrator"), NULL, NULL, LOGON_WITH_PROFILE, NULL, sPath, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);

        if (!ret) {
            ShowError();
            MessageBox(NULL, sPath, _T("��������ʧ��"), 0);
            return false;
        }
        WaitForSingleObject(pi.hThread, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    static void ShowError() {
        LPWSTR lpMessageBuf = NULL;
        //strerror(errno);
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);
    }

    static bool WriteStartupDir(const CString& strPath) {
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        return CopyFile(sPath, strPath, FALSE);
    }

    static bool WriteRegisterTable(const CString& strPath) {
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        TCHAR sPath[MAX_PATH] = _T("");
        GetModuleFileName(NULL, sPath, MAX_PATH);
        BOOL ret = CopyFile(sPath, strPath, FALSE);
        if (ret == false) {
            MessageBox(NULL, _T("�����ļ�ʧ�ܣ��Ƿ�Ȩ�޲���\r\n"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }

        HKEY hKey = NULL;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ�����ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }

        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_EXPAND_SZ, (BYTE*)(LPCTSTR)strPath, strPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            MessageBox(NULL, _T("�����Զ�����ʧ�ܣ��Ƿ�Ȩ�޲��㣿\r\n��������ʧ�ܣ�"), _T("����"), MB_ICONERROR | MB_TOPMOST);
            return false;
        }
        RegCloseKey(hKey);
        return true;
    }

    static bool Init() {
        HMODULE hModule = ::GetModuleHandle(nullptr);
        if (hModule == nullptr) {
            wprintf(L"����: GetModuleHandle ʧ��\n");
            return false;
        }

        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �ڴ˴�ΪӦ�ó������Ϊ��д���롣
            wprintf(L"����: MFC ��ʼ��ʧ��\n");
            return false;
        }
        return true;
    }
};
