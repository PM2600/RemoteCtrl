#pragma once

#include "DownLoadFileDlg.h"

// CFileMangerDlg 对话框

class CFileMangerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileMangerDlg)
private:
	CToolBar m_toolbar;
	CImageList toolbarImg;
	CImageList treeImg;
	CImageList listImg;
	CDownLoadFileDlg downLoadFileDlg;
	
private:
	void CreateToolBar();
	void GetDriveInfo();
	void InitMTree();
	void InitMList();
	void GetCurTreeItemPath(CString& path);
	void DelCurTreeItemChildItem(HTREEITEM hTree);
	void Time2CString(CString& strTime, __time64_t time);
	static void ThreadEntryDownLoadFile(void* arg);
	void ThreadDownLoadFile();
public:
	CFileMangerDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CFileMangerDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_MANAGER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CTreeCtrl m_Tree;
	CListCtrl m_List;
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void CmdDownLoadFile();
	afx_msg LRESULT OnShowDLDlg(WPARAM wParam,LPARAM lParam);
	afx_msg void CmdDeleteFile();
	virtual void OnCancel();
};
