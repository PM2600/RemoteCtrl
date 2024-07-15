// LockMachineDlg.cpp: 实现文件
//

#include "pch.h"
#include "SControlServer.h"
#include "LockMachineDlg.h"
#include "afxdialogex.h"


// CLockMachineDlg 对话框

IMPLEMENT_DYNAMIC(CLockMachineDlg, CDialogEx)

CLockMachineDlg::CLockMachineDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOCKMACHINEDLG, pParent)
{

}

CLockMachineDlg::~CLockMachineDlg()
{
}

void CLockMachineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCK_TXT_INFO, m_txtInfo);
}


BEGIN_MESSAGE_MAP(CLockMachineDlg, CDialogEx)
END_MESSAGE_MAP()


// CLockMachineDlg 消息处理程序
