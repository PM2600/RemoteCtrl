#pragma once
class CTool
{
public:
    static int AutoInvokeS(CStringW strExeName)
    {
        int ret = 0;
        BOOL isWow64;
        TCHAR tPath[MAX_PATH]{};
        CString strExePath;
        CString strSysPath;
        CString strSubKey = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");

        //0.����ϵͳ
        ret = IsWow64Process(GetCurrentProcess(), &isWow64);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("����ϵͳʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }

        //1.��õ�ǰEXE�ľ���·��
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("��õ�ǰEXE�ľ���·��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //4.��ע���
        HKEY hKey;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("��ע���ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        //5.��ѯע���
        long nKeyValueLen = MAX_PATH * sizeof(TCHAR);
        ret = RegQueryValue(hKey, strExeName, tPath, &nKeyValueLen);
        if (ret == ERROR_SUCCESS)
        {
            //�ҵ��ˣ���û��Ҫִ����
            return 1;
        }

        //6.��ȡϵͳĿ¼
        if (isWow64 != 0)
        {
            ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_SYSTEMX86, 0);
        }
        else
        {
            ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_SYSTEM, 0);
        }
        if (ret == 0)
        {
            //MessageBox(NULL, _T("��û�ȡϵͳĿ¼ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strSysPath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));
        strSysPath += _T("\\");
        strSysPath += strExeName;

        //7.������ϵͳĿ¼��
        ret = CopyFile(strExePath, strSysPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("������ϵͳĿ¼��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }

        //8.д��ע���
        ret = RegSetValueEx(hKey, strExeName, 0, REG_SZ, (BYTE*)(LPCTSTR)strSysPath, strSysPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("д��ע���ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    /// <summary>
    /// ����������
    /// </summary>
    static int AutoInvokeX(CStringW strExeName)
    {
        TCHAR tPath[MAX_PATH]{};
        CString strExePath;
        CString strStartupPath;
        int ret = 0;

        //1.��õ�ǰEXE�ľ���·��
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("��õ�ǰEXE�ľ���·��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.��ȡ����Ŀ¼
        ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_COMMON_STARTUP, 0);
        if (ret == FALSE)
        {
            //MessageBox(NULL, _T("��ȡ����Ŀ¼ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        strStartupPath = tPath;
        strStartupPath += _T("\\");
        strStartupPath += strExeName;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //3.�Ƿ��ƹ�
        if (PathFileExists(strStartupPath))
        {
            //���ƹ��ˣ���û��Ҫִ����
            return 1;
        }

        //4.���Ƶ�����Ŀ¼
        ret = CopyFile(strExePath, strStartupPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("����������Ŀ¼��ʧ��"), _T("����"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }

};

