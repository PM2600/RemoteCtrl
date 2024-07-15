#pragma once

#include "resource.h"

class CMToolbar
{
	CToolBar m_toolbar;
	CImageList img;
public:
	void Create(CWnd* pParent,int toolbarId)
	{
		//����ToolBar������
		if (!m_toolbar.CreateEx(pParent, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
			| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_toolbar.LoadToolBar(toolbarId))
			//IDR_TOOLBAR2 ��������Դid
		{
			TRACE0("Failed to Create Dialog ToolBar");
			return;
		}
		//��ʾ������
		pParent->RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
		//���ô�С
		CRect re;
		m_toolbar.GetWindowRect(&re);
		re.left = 370;
		re.top = 7;
		re.bottom = 60;
		re.right -= 35;
		m_toolbar.MoveWindow(&re);
		//����ͼ���б�  CImageList
		img.Create(25, 25, ILC_COLOR32 | ILC_MASK, 1, 1);//����ͼƬ��С��ͼƬ��ʽ��ͼƬ����
		//����ͼ��
		HICON hIcon = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_MOUSE), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon);
		HICON hIcon2 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_KEY), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon2);
		HICON hIcon3 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_LOCK), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon3);
		HICON hIcon4 = (HICON) ::LoadImage(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SW_UNLOCK), IMAGE_ICON, 64, 64, 0);
		img.Add(hIcon4);
		m_toolbar.GetToolBarCtrl().SetImageList(&img);
		//����
		m_toolbar.SetButtonText(0, L"������");
		m_toolbar.SetButtonText(2, L"���̿���");
		m_toolbar.SetButtonText(4, L"�����û�");
		m_toolbar.SetButtonText(6, L"�����û�");
	}
	//void AddItem(int imgId,)
};

