#pragma once


// CDownLoadFileDlg 对话框

class CDownLoadFileDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDownLoadFileDlg)

public:
	CDownLoadFileDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDownLoadFileDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLFILE_STATUS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_Info;
	CProgressCtrl m_Pro;
	CStatic m_TxtPro;
	CButton m_BtnOk;
	long long fileLen;
	int bfb;

	void SetFileLength(long long _fileLen);
	void SetDownLoadedLen(long long len);
	void SetInfo(CString& info);

	afx_msg void OnBnClickedBtnDlOk();
	virtual void OnCancel();
};
