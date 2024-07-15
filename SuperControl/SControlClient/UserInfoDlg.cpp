// UserInfoDlg.cpp: 实现文件
//

#include "pch.h"
#include "SControlClient.h"
#include "UserInfoDlg.h"
#include "afxdialogex.h"
#include "SControlClientDlg.h"


// CUserInfoDlg 对话框

IMPLEMENT_DYNAMIC(CUserInfoDlg, CDialogEx)

CUserInfoDlg::CUserInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_USER_INFO, pParent)
	, m_alias(_T(""))
	, m_username(_T(""))
	, m_os(_T(""))
	, m_runtime(_T(""))
	, m_ip(_T(""))
	, m_port(_T(""))
{

}

CUserInfoDlg::~CUserInfoDlg()
{
}

void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_alias);
	DDX_Text(pDX, IDC_EDIT2, m_username);
	DDX_Text(pDX, IDC_EDIT3, m_os);
	DDX_Text(pDX, IDC_EDIT4, m_runtime);
	DDX_Text(pDX, IDC_EDIT5, m_ip);
	DDX_Text(pDX, IDC_EDIT6, m_port);
}

void CUserInfoDlg::SetData(bool _isAdd, const CString& _m_alias, const CString& _m_username, 
	const CString& _m_os, const CString& _m_runtime, const CString& _m_ip, const CString& _m_port)
{
	isAdd = _isAdd;
	if (!_isAdd)
	{
		m_alias			= _m_alias;
		m_username		= _m_username;
		m_os			= _m_os;
		m_runtime		= _m_runtime;
		m_ip			= _m_ip;
		m_port			= _m_port;
	}
}


BEGIN_MESSAGE_MAP(CUserInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_OK, &CUserInfoDlg::OnBnClickedBtnOk)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CUserInfoDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CUserInfoDlg 消息处理程序


void CUserInfoDlg::OnBnClickedBtnOk()
{
	UpdateData(true);
	if (isAdd)
	{
		CSControlClientDlg* pParent = (CSControlClientDlg*)GetParent();
		pParent->AddListBeControlItem(m_alias, m_username, m_os, m_runtime, m_ip, m_port);
	}
	else
	{
		CSControlClientDlg* pParent = (CSControlClientDlg*)GetParent();
		pParent->ModListBeControlItem(m_alias, m_username, m_os, m_runtime, m_ip, m_port);
	}
	CDialogEx::OnOK();
}


void CUserInfoDlg::OnBnClickedBtnCancel()
{
	CDialogEx::OnCancel();
}


BOOL CUserInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
