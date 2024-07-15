// ScreenWatch.cpp: 实现文件
//

#include "pch.h"
#include "SControlClient.h"
#include "ScreenWatch.h"
#include "afxdialogex.h"
#include "ClientSocket.h"
#include "ClientController.h"
#include "Request.h"
#include "MThread.h"


// CScreenWatch 对话框

IMPLEMENT_DYNAMIC(CScreenWatch, CDialogEx)

void CScreenWatch::CreateToolBar()
{
	CMToolbar tol;

	//tol.Create(this, IDR_TOOLBAR2);

	//放到.h文件里
	//CToolBar m_toolbar;
	//CImageList img;
	//创建ToolBar工具条
	if (!m_toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_toolbar.LoadToolBar(IDR_TOOLBAR_SW))
		//IDR_TOOLBAR2 工具条资源id
	{
		TRACE0("Failed to Create Dialog ToolBar");
		EndDialog(IDCANCEL);
	}
	//显示工具栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	//设置大小
	CRect re;
	m_toolbar.GetWindowRect(&re);
	re.left = 370;
	re.top = 7;
	re.bottom = 60;
	re.right -= 35;
	m_toolbar.MoveWindow(&re);
	//创建图标列表  CImageList
	img.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//加载图片大小，图片格式，图片数量
	//加载图标
	HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_MOUSE), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon);
	HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_KEY), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon2);
	HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_LOCK), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon3);
	HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_UNLOCK), IMAGE_ICON, 64, 64, 0);
	img.Add(hIcon4);
	m_toolbar.GetToolBarCtrl().SetImageList(&img);
	//可选中
	m_toolbar.SetButtonInfo(0, ID_T_CMOUSE, TBBS_CHECKBOX, 0);
	m_toolbar.SetButtonInfo(2, ID_T_CKEY, TBBS_CHECKBOX, 1);
	//文字
	m_toolbar.SetButtonText(0, L"鼠标控制");
	m_toolbar.SetButtonText(2, L"键盘控制");
	m_toolbar.SetButtonText(4, L"锁定用户");
	m_toolbar.SetButtonText(6, L"解锁用户");

}

void CScreenWatch::ThreadEntryScreenWatch(void* arg)
{
	CScreenWatch* thiz = (CScreenWatch*)arg;
	thiz->ThreadScreenWatch();
	_endthread();
}

void CScreenWatch::ThreadScreenWatch()
{
	threadIsRunning = true;
	while (threadIsRunning)
	{
		//告诉服务器我要屏幕截图
		CClientSocket clientSock;
		clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
		clientSock.SetBufferSize(1024 * 1024 * 10);
		CPacket pack(5);
		clientSock.Send(pack);
		//得到截图
		TRACE("1=======================tick = %lld\r\n", GetTickCount64());
		int nCmd = clientSock.DealCommand();
		TRACE("2=======================tick = %lld\r\n", GetTickCount64());
		if (nCmd <= 0)
		{
			TRACE("获取屏幕截图错误(错误码: %d 错误 : % s)\r\n", GetLastError(), GetErrInfo(GetLastError()));
			AfxMessageBox(L"获取屏幕截图错误");
			return;
		}
		if (showOver == true)
		{
			//截图数据写入pStream
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
			if (hMem == NULL) return;
			IStream* pStream = NULL;
			HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
			ULONG written;
			pStream->Write(clientSock.GetPacket().sData.c_str(), clientSock.GetPacket().sData.size(), &written);
			//加载成图片
			image.Load(pStream);
			pStream->Release();
			GlobalFree(hMem);
			//
			showOver = false;

			TRACE("[threadId: %d] 成功读取一张照片 nCmd = %d\r\n", GetThreadId(GetCurrentThread()), nCmd);
			
		}
		clientSock.CloseSocket();
		//Sleep(30);
		//threadIsRunning = false;
	}
}

CPoint CScreenWatch::ClientPt2GlobalPt(CPoint& clientPt)
{
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);

	clientPt.x = 1.0 * clientPt.x * imageWidth / picRect.Width();
	clientPt.y = 1.0 * clientPt.y * imageHeight / picRect.Height();
	return clientPt;
}

CScreenWatch::CScreenWatch(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SCREEN_WATCH, pParent)
{
	showOver = true;
	isControlMouse = false;
	isControlKey = false;
	imageWidth = 0;
	imageHeight = 0;
}

CScreenWatch::~CScreenWatch()
{
}

void CScreenWatch::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_PIC_SCREEN, m_picScreen);
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CScreenWatch, CDialogEx)

	ON_WM_TIMER()
	ON_COMMAND(ID_T_CMOUSE, &CScreenWatch::CmdControlMouse)
	ON_COMMAND(ID_T_CKEY, &CScreenWatch::CmdControlKey)
	ON_COMMAND(ID_T_LOCKM, &CScreenWatch::CmdLockMachine)
	ON_COMMAND(ID_T_UNLOCKM, &CScreenWatch::CmdUnLockMachine)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON1, &CScreenWatch::OnBnClickedButton1)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


// CScreenWatch 消息处理程序


BOOL CScreenWatch::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CreateToolBar();
	hThread = (HANDLE)_beginthread(ThreadEntryScreenWatch, 0, this);
	SetTimer(123, 70, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CScreenWatch::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if ((nIDEvent == 123) && (showOver == false))
	{
		if (imageWidth == 0)
		{
			imageWidth = image.GetWidth();
		}
		if (imageHeight == 0)
		{
			imageHeight = image.GetHeight();
		}
		CRect picRect;
		m_picScreen.GetWindowRect(&picRect);
		CRect showPicRect(0, 0, picRect.Width(), picRect.Height());
		image.StretchBlt(m_picScreen.GetDC()->GetSafeHdc(), showPicRect, SRCCOPY);
		image.Save(L"D:\\Desktop\\1.png", Gdiplus::ImageFormatPNG);
		image.Destroy();
		showOver = true;
	}

	if (nIDEvent == 123)
	{
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CScreenWatch::OnCancel()
{
	threadIsRunning = false;
	WaitForSingleObject(hThread, 500);
	delete this;
}


void CScreenWatch::CmdControlMouse()
{
	isControlMouse = !isControlMouse;
}


void CScreenWatch::CmdControlKey()
{
	isControlKey = !isControlKey;
}


void CScreenWatch::CmdLockMachine()
{
	//发送给被控端
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	clientSock.SetBufferSize(1024 * 1024 * 2);
	CPacket pack(7);
	clientSock.Send(pack);
	int ret = clientSock.DealCommand();
	clientSock.CloseSocket();
}


void CScreenWatch::CmdUnLockMachine()
{
	//发送给被控端
	CClientSocket clientSock;
	clientSock.InitSocket(CClientController::m_vecUserInfos.at(0).ip, CClientController::m_vecUserInfos.at(0).port);
	clientSock.SetBufferSize(1024 * 1024 * 2);
	CPacket pack(8);
	clientSock.Send(pack);
	int ret = clientSock.DealCommand();
	clientSock.CloseSocket();
}

//左键双击
void CScreenWatch::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::DBCLICK;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处双击\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));
	//***
	CDialogEx::OnLButtonDblClk(nFlags, point);
}

//左键按下
void CScreenWatch::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::DOWN;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处按下\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	

	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnLButtonDown(nFlags, point);
}

//左键弹起
void CScreenWatch::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::LEFT;
	mouseInfo.nEvent = MOUSEEVE::UP;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 左键将在(%d,%d)处弹起\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnLButtonUp(nFlags, point);
}

//右键双击
void CScreenWatch::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::DBCLICK;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处双击\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonDblClk(nFlags, point);
}

//右键按下
void CScreenWatch::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::DOWN;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处按下\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonDown(nFlags, point);
}

//右键弹起
void CScreenWatch::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::RIGHT;
	mouseInfo.nEvent = MOUSEEVE::UP;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 右键将在(%d,%d)处弹起\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	
	//发送给被控端
	req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	//***
	CDialogEx::OnRButtonUp(nFlags, point);
}

void CScreenWatch::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);
}



//鼠标移动
void CScreenWatch::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!isControlMouse) return;
	if (imageWidth == 0 || imageHeight == 0) return;
	//把point的Y坐标的原点移动到，pic控件的原点上
	CRect picRect;
	m_picScreen.GetWindowRect(&picRect);
	ScreenToClient(&picRect);
	point.y -= picRect.top;
	if (point.y < 0) return;
	//组织鼠标信息
	MOUSEINFO mouseInfo;
	mouseInfo.nButton = MOUSEBTN::NOTHING;
	mouseInfo.nEvent = MOUSEEVE::MOVE;
	mouseInfo.ptXY = ClientPt2GlobalPt(point);
	TRACE("[threadId: %d] 鼠标将要移动到(%d,%d)\r\n", GetThreadId(GetCurrentThread()), mouseInfo.ptXY.x, mouseInfo.ptXY.y);
	//发送给被控端
	req.SendPacket( 6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));

	TRACE("OnMouseMove %lld\r\n", GetTickCount64());

	//***
	CDialogEx::OnMouseMove(nFlags, point);
}

//**********************************************************************
//
// Sends Win + D to toggle to the desktop
//
//**********************************************************************
void CScreenWatch::ShowDesktop(DWORD code)
{
	CPoint p(1047, 30);
	ClientToScreen(&p);

	INPUT inputs1[2] = {};
	ZeroMemory(inputs1, sizeof(inputs1));

	inputs1[0].type = INPUT_MOUSE;
	inputs1[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	inputs1[0].mi.dwExtraInfo = GetMessageExtraInfo();

	inputs1[1].type = INPUT_MOUSE;
	inputs1[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
	inputs1[1].mi.dwExtraInfo = GetMessageExtraInfo();

	UINT uSent;

	INPUT inputs[2] = {};

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = code;

	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki.wVk = code/*VK_SCROLL*/;
	inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

	int nSize = ARRAYSIZE(inputs);
	uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	if (uSent != ARRAYSIZE(inputs))
	{
	}
}


void CScreenWatch::OnBnClickedButton1()
{
	for (int i = 0; i < 1000; i++)
	{
		MOUSEINFO mouseInfo;
		mouseInfo.nButton = MOUSEBTN::RIGHT;
		mouseInfo.nEvent = MOUSEEVE::UP;
		mouseInfo.ptXY = CPoint(100,100 + i);
		req.SendPacket(6, (BYTE*)&mouseInfo, sizeof(MOUSEINFO));
	}

	

}


void CScreenWatch::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CScreenWatch::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (pMsg->message == WM_KEYDOWN)
	{
	}
	
	return CDialogEx::PreTranslateMessage(pMsg);
}
