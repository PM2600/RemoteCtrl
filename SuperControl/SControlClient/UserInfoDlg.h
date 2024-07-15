#pragma once


// CUserInfoDlg 对话框

class CUserInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUserInfoDlg)




private:
public:
	CUserInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CUserInfoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_USER_INFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	// 标识
	CString m_alias;
	// 用户名
	CString m_username;
	// 操作系统
	CString m_os;
	// 运行时间
	CString m_runtime;
	// ip地址
	CString m_ip;
	// 端口
	CString m_port;
	//添加/修改
	bool isAdd;

	void SetData(bool _isAdd = true, const CString& _m_alias = nullptr, const CString& _m_username = NULL, const CString& _m_os = NULL, const CString& _m_runtime = NULL, const CString& _m_ip = NULL, const CString& _m_port = NULL);
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
	virtual BOOL OnInitDialog();
};
