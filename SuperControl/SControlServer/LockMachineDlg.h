#pragma once

#include <afxdialogex.h>

// CLockMachineDlg 对话框

class CLockMachineDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLockMachineDlg)

public:
	CLockMachineDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CLockMachineDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOCKMACHINEDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_txtInfo;
};
