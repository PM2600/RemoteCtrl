// FileMangerDlg.cpp: 实现文件
//

#include "pch.h"
#include "SControlClient.h"
#include "FileMangerDlg.h"
#include "afxdialogex.h"
#include "ClientSocket.h"
#include "ClientController.h"

#define WM_SHOWDLDLG (WM_USER + 1)

// CFileMangerDlg 对话框

IMPLEMENT_DYNAMIC(CFileMangerDlg, CDialogEx)

CFileMangerDlg::CFileMangerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILE_MANAGER, pParent)
{

}

CFileMangerDlg::~CFileMangerDlg()
{
}

void CFileMangerDlg::CreateToolBar()
{
	//创建ToolBar工具条
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_FM))
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	//显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置大小
	CRect re;
	CRect mTreeRect;
	CRect clientRect;
	m_toolbar.GetWindowRect(&re);
	m_Tree.GetWindowRect(&mTreeRect);
	GetWindowRect(&clientRect);
	re.top = 7;
	re.left = mTreeRect.Width() + 15;
	re.right = clientRect.Width() - 35;
	re.bottom = 60;
	m_toolbar.MoveWindow(&re);
	if (toolbarImg.m_hImageList == NULL)
	{
		//创建图标列表  CImageList
		toolbarImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
		//加载图标
		HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_SHW), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon);
		HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_MOD), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon2);
		HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_SET), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon3);
		HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DEL), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon4);
		HICON hIcon5 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DOW), IMAGE_ICON, 64, 64, 0);
		toolbarImg.Add(hIcon5);
	}
	m_toolbar.GetToolBarCtrl().SetImageList(&toolbarImg);
	//文字
	m_toolbar.SetButtonText(0, L"查看文件");
	m_toolbar.SetButtonText(2, L"修改文件");
	m_toolbar.SetButtonText(4, L"设置文件");
	m_toolbar.SetButtonText(6, L"删除文件");
	m_toolbar.SetButtonText(8, L"下载文件");
}

/// <summary>
/// 从被控端拿驱动信息
/// </summary>
void CFileMangerDlg::GetDriveInfo()
{
	//拿驱动信息
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(1);
	clientSock.Send(pack);
	int nCmd = clientSock.DealCommand();
	clientSock.CloseSocket();
	if (nCmd <= 0)
	{
		TRACE("接受驱动信息错误(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"接受驱动信息错误");
	}
	DRIVEINFO driveInfo;
	memcpy(&driveInfo, clientSock.GetPacket().sData.c_str(), clientSock.GetPacket().sData.size());
	//加载到树中
	for (int i = 0; i < driveInfo.drive_count; i++)
	{
		CString str;
		str.Format(L"%c:", driveInfo.drive[i]);
		HTREEITEM hTree = m_Tree.InsertItem(str, 0, 0, TVI_ROOT);
	}
}

void CFileMangerDlg::InitMTree()
{
	treeImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_DRIVE), IMAGE_ICON, 64, 64, 0);
	treeImg.Add(hIcon);
	HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DIR), IMAGE_ICON, 64, 64, 0);
	treeImg.Add(hIcon2);
	m_Tree.SetImageList(&treeImg, TVSIL_NORMAL);
}

void CFileMangerDlg::InitMList()
{

	listImg.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_FM_FILE), IMAGE_ICON, 64, 64, 0);
	listImg.Add(hIcon);
	m_List.SetImageList(&listImg, 1);

	m_List.InsertColumn(0, _T("文件名称"), 0, 150);
	m_List.InsertColumn(1, _T("创建时间"), 0, 170);
	m_List.InsertColumn(2, _T("上次访问"), 0, 170);
	m_List.InsertColumn(3, _T("上次修改"), 0, 170);
	m_List.InsertColumn(4, _T("文件长度"), 0, 150);

	DWORD extStyle = m_List.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;
	extStyle |= LVS_EX_FULLROWSELECT;
	m_List.SetExtendedStyle(extStyle);
}



void CFileMangerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
}

BEGIN_MESSAGE_MAP(CFileMangerDlg, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CFileMangerDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CFileMangerDlg::OnNMDblclkTreeDir)
	ON_COMMAND(ID_T_DOWN, &CFileMangerDlg::CmdDownLoadFile)
	ON_MESSAGE(WM_SHOWDLDLG, OnShowDLDlg)
	ON_COMMAND(ID_T_DEL, &CFileMangerDlg::CmdDeleteFile)
END_MESSAGE_MAP()


// CFileMangerDlg 消息处理程序


BOOL CFileMangerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CreateToolBar();
	InitMTree();
	InitMList();

	GetDriveInfo();


	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CFileMangerDlg::GetCurTreeItemPath(CString& path)
{
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	do
	{
		path = m_Tree.GetItemText(hTree) + "\\" + path;
		TRACE("%S\r\n", path.GetBuffer());
	} while ((hTree = m_Tree.GetParentItem(hTree)) != NULL);
}

void CFileMangerDlg::DelCurTreeItemChildItem(HTREEITEM hTree)
{
	HTREEITEM hSubTree;
	while ((hSubTree = m_Tree.GetChildItem(hTree)) != NULL)
	{
		m_Tree.DeleteItem(hSubTree);
	}
}

void CFileMangerDlg::Time2CString(CString& strTime, __time64_t time)
{
	if (time == -1) {
		strTime = _T("??-??-??:??:??:??");
	}
	else {
		char str[0xFF]{};
		tm t;
		localtime_s(&t, &time);
		int ret = strftime(str, 0xff, "%Y-%m-%d:%H:%M:%S", &t);
		TCHAR wideTime[MAX_PATH]{};
		int transRet = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), wideTime, MAX_PATH);
		strTime = wideTime;
	}
}

void CFileMangerDlg::ThreadEntryDownLoadFile(void* arg)
{
	CFileMangerDlg* thiz = (CFileMangerDlg*)arg;
	thiz->ThreadDownLoadFile();
	_endthread();
}

void CFileMangerDlg::ThreadDownLoadFile()
{
	//要下载的文件
	CString path;
	GetCurTreeItemPath(path);
	CString name = m_List.GetItemText(m_List.GetSelectionMark(), 0);
	char multiPath[MAX_PATH]{};
	char multiName[MAX_PATH]{};
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	transRet = WideCharToMultiByte(CP_ACP, 0, name.GetBuffer(), -1, multiName, MAX_PATH, NULL, NULL);
	strcat(multiPath, multiName);
	//要保存到哪里去
	CFileDialog fileDlg(false, 0, name);
	if (fileDlg.DoModal() != IDOK)
	{
		AfxMessageBox(L"未选择要保存");
		return;
	}
	char multiSavePath[MAX_PATH]{};
	transRet = WideCharToMultiByte(CP_ACP, 0, fileDlg.GetPathName().GetBuffer(), -1, multiSavePath, MAX_PATH, NULL, NULL);
	FILE* pFile = fopen(multiSavePath, "wb+");
	if (pFile == NULL)
	{
		AfxMessageBox(L"文件无法创建，可能没有权限");
		return;
	}

	//弹出进度
	SendMessage(WM_SHOWDLDLG);

	CString info;
	info.Format(L"[开始下载...]\r\n[从 %s 下载]\r\n[到 %s 保存]", (path + name).GetBuffer(), fileDlg.GetPathName().GetBuffer());
	downLoadFileDlg.SetInfo(info);

	//告诉服务器我要下载文件
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(3, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);
	//接收文件长度
	int nCmd = clientSock.DealCommand();
	if (nCmd <= 0)
	{
		TRACE("下载文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"下载文件错误");
		return;
	}
	long long fileLen = *(long long*)clientSock.GetPacket().sData.c_str();
	if (fileLen <= 0)
	{
		TRACE("文件长度为零，或者没有权限(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"文件长度为零，或者没有权限");
		return;
	}
	downLoadFileDlg.SetFileLength(fileLen);

	//接收文件
	long long readedLen = 0;
	while (readedLen < fileLen)
	{
		nCmd = clientSock.DealCommand();
		int readLen = clientSock.GetPacket().sData.size();
		if (nCmd <= 0)
		{
			TRACE("下载文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"下载文件错误");
			return;
		}
		int writeLen = fwrite(clientSock.GetPacket().sData.c_str(), 1, readLen, pFile);
		if (writeLen <= 0)
		{
			TRACE("文件写入错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"文件写入错误");
			return;
		}
		readedLen += readLen;
		downLoadFileDlg.SetDownLoadedLen(readedLen);
	}
	//关闭
	clientSock.CloseSocket();
	fclose(pFile);

}


void CFileMangerDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
}


void CFileMangerDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;

	CString path;
	GetCurTreeItemPath(path);
	char multiPath[MAX_PATH]{};
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(2, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);
	int nCmd = clientSock.DealCommand();
	PFILEINFO pFileInfo = (PFILEINFO)clientSock.GetPacket().sData.c_str();
	HTREEITEM hTree = m_Tree.GetSelectedItem();
	DelCurTreeItemChildItem(hTree);
	m_List.DeleteAllItems();
	while (!pFileInfo->isNull)
	{
		if (nCmd <= 0)
		{
			TRACE("接受文件信息错误(错误码:%d 错误:%s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"接受文件信息错误");
			return;
		}
		TRACE("nCmd = %d name = %s\r\n", nCmd, pFileInfo->data.name);
		TCHAR str[MAX_PATH]{};
		int transRet = MultiByteToWideChar(CP_ACP, 0, pFileInfo->data.name, strlen(pFileInfo->data.name), str, MAX_PATH);
		if ((strcmp(pFileInfo->data.name, ".") == 0) || (strcmp(pFileInfo->data.name, "..") == 0))
		{
		}
		else
		{
			if (pFileInfo->data.attrib & _A_SUBDIR)
			{
				m_Tree.InsertItem(str, 1, 1, hTree);
			}
			else
			{
				int index = m_List.GetItemCount();
				CString strTime;
				m_List.InsertItem(index, str, 0);
				Time2CString(strTime, pFileInfo->data.time_create);
				m_List.SetItemText(index, 1, strTime);
				Time2CString(strTime, pFileInfo->data.time_access);
				m_List.SetItemText(index, 2, strTime);
				Time2CString(strTime, pFileInfo->data.time_write);
				m_List.SetItemText(index, 3, strTime);
				if (pFileInfo->data.size < 1024)
				{
					strTime.Format(L"%dB", pFileInfo->data.size);
				}
				else
				{
					strTime.Format(L"%dK", (int)(1.0 * pFileInfo->data.size / 1024));
				}
				m_List.SetItemText(index, 4, strTime);
			}
		}
		nCmd = clientSock.DealCommand();
		pFileInfo = (PFILEINFO)clientSock.GetPacket().sData.c_str();
	}

	clientSock.CloseSocket();

}


void CFileMangerDlg::CmdDownLoadFile()
{
	if (m_Tree.GetSelectedItem() < 0 || m_List.GetSelectionMark() < 0) return;
	_beginthread(ThreadEntryDownLoadFile, 0, this);
}

LRESULT CFileMangerDlg::OnShowDLDlg(WPARAM wParam, LPARAM lParam)
{
	downLoadFileDlg.Create(IDD_DLFILE_STATUS);
	downLoadFileDlg.ShowWindow(SW_SHOW);
	downLoadFileDlg.SetActiveWindow();
	downLoadFileDlg.CenterWindow();

	return 0;
}


void CFileMangerDlg::CmdDeleteFile()
{
	if (m_Tree.GetSelectedItem() < 0 || m_List.GetSelectionMark() < 0) return;
	//要删除的文件
	CString path;
	GetCurTreeItemPath(path);
	CString name = m_List.GetItemText(m_List.GetSelectionMark(), 0);
	char multiPath[MAX_PATH]{};
	char multiName[MAX_PATH]{};
	int transRet = WideCharToMultiByte(CP_ACP, 0, path.GetBuffer(), -1, multiPath, MAX_PATH, NULL, NULL);
	transRet = WideCharToMultiByte(CP_ACP, 0, name.GetBuffer(), -1, multiName, MAX_PATH, NULL, NULL);
	strcat(multiPath, multiName);
	//告诉服务器我要删除文件
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	CPacket pack(4, (BYTE*)multiPath, strlen(multiPath));
	clientSock.Send(pack);
	//是否成功
	int nCmd = clientSock.DealCommand();
	if (nCmd <= 0)
	{
		TRACE("删除文件错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
		AfxMessageBox(L"删除文件错误");
		return;
	}
	int success = *(int*)clientSock.GetPacket().sData.c_str();
	if (success)
	{
		m_List.DeleteItem(m_List.GetSelectionMark());
	}
}


void CFileMangerDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialogEx::OnCancel();
}
