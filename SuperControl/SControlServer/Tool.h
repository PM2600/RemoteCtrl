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

        //0.区分系统
        ret = IsWow64Process(GetCurrentProcess(), &isWow64);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("区分系统失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }

        //1.获得当前EXE的绝对路径
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("获得当前EXE的绝对路径失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //4.打开注册表
        HKEY hKey;
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("打开注册表失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        //5.查询注册表
        long nKeyValueLen = MAX_PATH * sizeof(TCHAR);
        ret = RegQueryValue(hKey, strExeName, tPath, &nKeyValueLen);
        if (ret == ERROR_SUCCESS)
        {
            //找到了，就没必要执行了
            return 1;
        }

        //6.获取系统目录
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
            //MessageBox(NULL, _T("获得获取系统目录失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strSysPath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));
        strSysPath += _T("\\");
        strSysPath += strExeName;

        //7.拷贝到系统目录下
        ret = CopyFile(strExePath, strSysPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("拷贝到系统目录下失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }

        //8.写入注册表
        ret = RegSetValueEx(hKey, strExeName, 0, REG_SZ, (BYTE*)(LPCTSTR)strSysPath, strSysPath.GetLength() * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS)
        {
            //MessageBox(NULL, _T("写入注册表失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }
    /// <summary>
    /// 开机自启动
    /// </summary>
    static int AutoInvokeX(CStringW strExeName)
    {
        TCHAR tPath[MAX_PATH]{};
        CString strExePath;
        CString strStartupPath;
        int ret = 0;

        //1.获得当前EXE的绝对路径
        ret = GetModuleFileName(NULL, tPath, MAX_PATH);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("获得当前EXE的绝对路径失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strExePath = tPath;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //2.获取启动目录
        ret = SHGetSpecialFolderPath(NULL, tPath, CSIDL_COMMON_STARTUP, 0);
        if (ret == FALSE)
        {
            //MessageBox(NULL, _T("获取启动目录失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        strStartupPath = tPath;
        strStartupPath += _T("\\");
        strStartupPath += strExeName;
        memset(tPath, 0, MAX_PATH * sizeof(TCHAR));

        //3.是否复制过
        if (PathFileExists(strStartupPath))
        {
            //复制过了，就没必要执行了
            return 1;
        }

        //4.复制到启动目录
        ret = CopyFile(strExePath, strStartupPath, FALSE);
        if (ret == 0)
        {
            //MessageBox(NULL, _T("拷贝到启动目录下失败"), _T("错误"), MB_ICONERROR);
            return -1;
        }
        return 1;
    }

};

