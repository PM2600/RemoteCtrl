#pragma once
#include "MToolbar.h"
#include "Request.h"

// CScreenWatch 对话框

class CScreenWatch : public CDialogEx
{
	DECLARE_DYNAMIC(CScreenWatch)

private:
	CRequest req;
	CToolBar m_toolbar;
	CImageList img;
	bool isControlMouse;
	bool isControlKey;
	bool showOver;
	CImage image;
	int		imageWidth;
	int		imageHeight;
	CStatic m_picScreen;
	void CreateToolBar();
	HANDLE hThread;
	bool threadIsRunning;
	static void ThreadEntryScreenWatch(void* arg);
	void ThreadScreenWatch();
	CPoint ClientPt2GlobalPt(CPoint& clientPt);
	void ShowDesktop(DWORD code);

public:
	CScreenWatch(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CScreenWatch();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREEN_WATCH };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void OnCancel();
	afx_msg void CmdControlMouse();
	afx_msg void CmdControlKey();
	afx_msg void CmdLockMachine();
	afx_msg void CmdUnLockMachine();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CEdit m_edit;
};
