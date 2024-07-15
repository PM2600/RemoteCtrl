
// SControlClientDlg.h: 头文件
//

#pragma once

#include "UserInfoDlg.h"
#include "FileMangerDlg.h"
#include "FileMangerDlg.h"
#include "ScreenWatch.h"
#include "UDPPassClient.h"

#define WM_ONLINE (WM_USER + 10)

// CSControlClientDlg 对话框
class CSControlClientDlg : public CDialogEx
{

private:
	CToolBar m_toolbar;
	CImageList img;
	CImageList img2;
	CFileMangerDlg* fileManagerDlg;
	CScreenWatch* screenWatch;

private:
	void CreateToolBar();
	void InitListBeControl();
	void InitListInfo();
	
private:

public:
	afx_msg void CmdAddUser();
	afx_msg void CmdModUser();
	afx_msg void CmdDelUser();

	void AddListBeControlItem(const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);
	void ModListBeControlItem(const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);


// 构造
public:
	CSControlClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCONTROLCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnOnLine(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl					m_ListBeControl;
	CListCtrl					m_ListInfo;
	UDPPassClient				m_udppass;
	std::vector<long long>		m_ids;

	afx_msg void CmdFileManager();
	afx_msg void CmdScreenWatch();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
