
// SControlClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SControlClient.h"
#include "SControlClientDlg.h"
#include "afxdialogex.h"
#include "ScreenWatch.h"
#include "ClientController.h"
#include "Tool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

END_MESSAGE_MAP()


// CSControlClientDlg 对话框



void CSControlClientDlg::CreateToolBar()
{
	//创建ToolBar工具条
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_MAIN))
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	//显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置大小
	CRect re;
	m_toolbar.GetWindowRect(&re);
	re.top = 0;
	re.bottom = 80;
	m_toolbar.MoveWindow(&re);
	//创建图标列表  CImageList
	img.Create(45, 45, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DIR), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon);
	HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SCREEN), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon2);
	HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_ADD), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon3);
	HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_MOD), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon4);
	HICON hIcon5 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_DEL), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon5);
	m_toolbar.GetToolBarCtrl().SetImageList(&img);
	//文字
	m_toolbar.SetButtonText(0, L"文件管理");
	m_toolbar.SetButtonText(2, L"屏幕监控");
	m_toolbar.SetButtonText(4, L"添加客户");
	m_toolbar.SetButtonText(6, L"修改客户");
	m_toolbar.SetButtonText(8, L"删除客户");
}

void CSControlClientDlg::InitListBeControl()
{
	m_ListBeControl.InsertColumn(0, _T("标识"), 0, 150);
	m_ListBeControl.InsertColumn(1, _T("用户名"), 0, 150);
	m_ListBeControl.InsertColumn(2, _T("操作系统"), 0, 150);
	m_ListBeControl.InsertColumn(3, _T("运行时间"), 0, 150);
	m_ListBeControl.InsertColumn(4, _T("IP"), 0, 150);
	m_ListBeControl.InsertColumn(5, _T("PORT"), 0, 150);


	DWORD extStyle = m_ListBeControl.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;
	extStyle |= LVS_EX_FULLROWSELECT;
	m_ListBeControl.SetExtendedStyle(extStyle);

	int i = 0;
	m_ListBeControl.InsertItem(i,	  _T("本机"));
	m_ListBeControl.SetItemText(i, 1, _T("Administrator"));
	m_ListBeControl.SetItemText(i, 2, _T("WIN10"));
	m_ListBeControl.SetItemText(i, 3, _T("10:59:43"));
	m_ListBeControl.SetItemText(i, 4, _T("127.0.0.1"));
	m_ListBeControl.SetItemText(i, 5, _T("6888"));
	i = 1;
	m_ListBeControl.InsertItem(i,     _T("被控机"));
	m_ListBeControl.SetItemText(i, 1, _T("Administrator"));
	m_ListBeControl.SetItemText(i, 2, _T("WIN10"));
	m_ListBeControl.SetItemText(i, 3, _T("10:59:43"));
	m_ListBeControl.SetItemText(i, 4, _T("192.168.88.141"));
	m_ListBeControl.SetItemText(i, 5, _T("6888"));
}

void CSControlClientDlg::InitListInfo()
{
	img2.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量

	HICON hIcon3 = NULL;

	hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_MSG), IMAGE_ICON, 64, 64, 0);
	img2.Add(hIcon3);

	m_ListInfo.SetImageList(&img2, 1);
	m_ListInfo.InsertColumn(0, _T(""), 0, 33);
	m_ListInfo.InsertColumn(1, _T("时间"), 0, 150);
	m_ListInfo.InsertColumn(2, _T("消息"), 0, 600);

	DWORD extStyle = m_ListInfo.GetExtendedStyle();
	extStyle |= LVS_EX_GRIDLINES;
	extStyle |= LVS_EX_FULLROWSELECT;
	m_ListInfo.SetExtendedStyle(extStyle);
}

void CSControlClientDlg::AddListBeControlItem(const CString& _m_alias, const CString& _m_username, const CString& _m_os,
	const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	int i = m_ListBeControl.GetItemCount();
	m_ListBeControl.InsertItem(i, _m_alias);
	m_ListBeControl.SetItemText(i, 1, _m_username);
	m_ListBeControl.SetItemText(i, 2, _m_os);
	m_ListBeControl.SetItemText(i, 3, _m_runtime);
	m_ListBeControl.SetItemText(i, 4, _m_ip);
	m_ListBeControl.SetItemText(i, 5, _m_port);
}

void CSControlClientDlg::ModListBeControlItem(const CString& _m_alias, const CString& _m_username, const CString& _m_os,
	const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	int i = m_ListBeControl.GetSelectionMark();
	m_ListBeControl.SetItemText(i, 0, _m_alias);
	m_ListBeControl.SetItemText(i, 1, _m_username);
	m_ListBeControl.SetItemText(i, 2, _m_os);
	m_ListBeControl.SetItemText(i, 3, _m_runtime);
	m_ListBeControl.SetItemText(i, 4, _m_ip);
	m_ListBeControl.SetItemText(i, 5, _m_port);
}

void CSControlClientDlg::CmdAddUser()
{
	CUserInfoDlg userInfoDlg(this);
	userInfoDlg.SetData();
	userInfoDlg.DoModal();
}

void CSControlClientDlg::CmdModUser()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	CString alias = m_ListBeControl.GetItemText(i, 0);
	CString username = m_ListBeControl.GetItemText(i, 1);
	CString os = m_ListBeControl.GetItemText(i, 2);
	CString runtime = m_ListBeControl.GetItemText(i, 3);
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	CUserInfoDlg userInfoDlg(this);
	userInfoDlg.SetData(false, alias, username, os, runtime, ip, port);
	userInfoDlg.DoModal();
}

void CSControlClientDlg::CmdDelUser()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	m_ListBeControl.DeleteItem(i);
}

CSControlClientDlg::CSControlClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCONTROLCLIENT_DIALOG, pParent)
	, m_udppass("192.168.88.149", 16888, 18888)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSControlClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_BECONTROL, m_ListBeControl);
	DDX_Control(pDX, IDC_LIST_INFO, m_ListInfo);
}

BEGIN_MESSAGE_MAP(CSControlClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_T_ADDUSER, CSControlClientDlg::CmdAddUser)
	ON_COMMAND(ID_T_MODUSER, CSControlClientDlg::CmdModUser)
	ON_COMMAND(ID_T_DELUSER, CSControlClientDlg::CmdDelUser)
	ON_COMMAND(ID_T_FILEMANAGER, &CSControlClientDlg::CmdFileManager)
	ON_COMMAND(ID_T_SCREENWATCH, &CSControlClientDlg::CmdScreenWatch)
	ON_MESSAGE(WM_ONLINE, &CSControlClientDlg::OnOnLine)
	ON_BN_CLICKED(IDC_BUTTON1, &CSControlClientDlg::OnBnClickedButton1)//UDP连接
	ON_BN_CLICKED(IDC_BUTTON2, &CSControlClientDlg::OnBnClickedButton2)//UDP消息发送
END_MESSAGE_MAP()


// CSControlClientDlg 消息处理程序

BOOL CSControlClientDlg::OnInitDialog()
{


	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	CreateToolBar();
	InitListBeControl();
	InitListInfo();

	m_udppass.Invoke(GetSafeHwnd());

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

LRESULT CSControlClientDlg::OnOnLine(WPARAM wParam, LPARAM lParam)
{

	m_ListBeControl.DeleteAllItems();
	m_ListInfo.DeleteAllItems();
	if (wParam == 1)
	{
		return LRESULT();
	}

	std::map<long long, MUserInfo> mapAddrs = m_udppass.GetMapAddrs();
	//显示到(信息提示列表中)
	int count0 = m_ListInfo.GetItemCount();
	int count1 = m_ListBeControl.GetItemCount();
	int i = 0;
	for (std::map<long long, MUserInfo>::iterator it = mapAddrs.begin();it != mapAddrs.end();it ++)
	{
		int idx = i + count0;
		m_ListInfo.InsertItem(LVIF_TEXT | LVIF_STATE, idx , L"", 0, LVIS_SELECTED, 0, 0);
		m_ListInfo.SetItemText(idx, 1, CTool::GetCurrentTime());
		MUserInfo& mInfo = it->second;
		WCHAR wideIp[32]{};
		MultiByteToWideChar(CP_ACP, 0, mInfo.ip, 16, wideIp, 32);
		CString ip = wideIp;
		ip += L" 已经上线";
		m_ListInfo.SetItemText(idx, 2, ip);

		i++;
	}
	i = 0;
	//清空ids
	m_ids.clear();
	//显示到(控制列表中)
	for (std::map<long long, MUserInfo>::iterator it = mapAddrs.begin(); it != mapAddrs.end(); it++)
	{	
		m_ids.push_back(it->first);
		MUserInfo& mInfo = it->second;
		WCHAR wideIp[32]{};
		MultiByteToWideChar(CP_ACP, 0, mInfo.ip, 16, wideIp, 32);

		CString port;
		port.Format(_T("%d"), mInfo.port);
		int idx = i + count1;
		m_ListBeControl.InsertItem(idx,     _T("___"));
		m_ListBeControl.SetItemText(idx, 1, _T("未知"));
		m_ListBeControl.SetItemText(idx, 2, _T("未知"));
		m_ListBeControl.SetItemText(idx, 3, _T("未知"));
		m_ListBeControl.SetItemText(idx, 4, wideIp);
		m_ListBeControl.SetItemText(idx, 5, port);

		i++;
	}

	return LRESULT();
}

void CSControlClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

//第一版完成！！！！！哦耶！！！！


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSControlClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSControlClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSControlClientDlg::CmdFileManager()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	if (CClientController::m_vecUserInfos.size() > 0)
	{
		CClientController::m_vecUserInfos.pop_back();
	}
	CClientController::m_vecUserInfos.push_back(USERINFO(ip, port));

	fileManagerDlg = new CFileMangerDlg;
	fileManagerDlg->Create(IDD_FILE_MANAGER);
	fileManagerDlg->ShowWindow(SW_SHOW);
}


void CSControlClientDlg::CmdScreenWatch()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	CString ip = m_ListBeControl.GetItemText(i, 4);
	CString port = m_ListBeControl.GetItemText(i, 5);
	if (CClientController::m_vecUserInfos.size() > 0)
	{
		CClientController::m_vecUserInfos.pop_back();
	}
	CClientController::m_vecUserInfos.push_back(USERINFO(ip, port));
	screenWatch = new CScreenWatch;
	screenWatch->Create(IDD_SCREEN_WATCH);
	screenWatch->ShowWindow(SW_SHOW);
}


void CSControlClientDlg::OnBnClickedButton1()
{
	int i = m_ListBeControl.GetSelectionMark();
	if (i == -1) return;
	if (m_ids.size() > 0) {
		m_udppass.RequestConnect(m_ids.at(i));
	}
}


void CSControlClientDlg::OnBnClickedButton2()
{
	m_udppass.SentToBeCtrl();
}
