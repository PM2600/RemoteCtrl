#pragma once

#include "MQueue.h"
#include "Common.h"
#include "MThread.h"
#include <atlimage.h>

class CScreenshot : public CMFuncBase
{
private:
	//隔开一定时间，启动俩个线程
	int Invoke()
	{
		for (int i = 0; i < 2; i++)
		{
			m_pool.DispatchWork(CMWork(this, (MT_FUNC)&CScreenshot::Screen));
			Sleep(10);
		}
		return -1;
	}
	//截图
	int Screen()
	{
		Sleep(10);
		CPacket screenPacket;
		ScreenWatch(screenPacket);
		if (m_queScreenPackets.size() >= 10)
		{
			CPacket p;
			m_queScreenPackets.pop_front(p);
		}		
		m_queScreenPackets.push_back(screenPacket);
		printf("[Thread Id : %08X] {Tick : %lld}<------截图-----> count : %d\r\n",GetThreadId(GetCurrentThread()),GetTickCount64(), m_queScreenPackets.size());
		return 0;
	}
	//截屏功能
	void ScreenWatch(CPacket& screenPacket)
	{
		CImage screen;
		HDC hScreen = ::GetDC(NULL);
		int nBitperPixel = GetDeviceCaps(hScreen, BITSPIXEL);
		int nWidth = GetDeviceCaps(hScreen, HORZRES);
		int nHeight = GetDeviceCaps(hScreen, VERTRES);
		screen.Create(nWidth, nHeight, nBitperPixel);
		BitBlt(screen.GetDC(), 0, 0, nWidth, nHeight, hScreen, 0, 0, SRCCOPY);

		ReleaseDC(NULL, hScreen);

		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hMem == NULL) return;
		IStream* pStream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
		

		if (ret == S_OK)
		{
			screen.Save(pStream, Gdiplus::ImageFormatPNG);
			LARGE_INTEGER li = { 0 };
			pStream->Seek(li, STREAM_SEEK_SET, NULL);
			LPVOID pData = GlobalLock(hMem);
			screenPacket = CPacket(5, (BYTE*)pData, GlobalSize(hMem));

			GlobalUnlock(hMem);
		}
		GlobalFree(hMem);
		screen.ReleaseDC();
		screen.Destroy();
	}
public:
	//线程安全队列
	CMQueue<CPacket> m_queScreenPackets;
	//线程池
	CMThreadPool     m_pool;
	CScreenshot() : m_pool(3)
	{
		m_pool.Invoke();
		m_pool.DispatchWork(CMWork(this, (MT_FUNC)&CScreenshot::Invoke));
	}
	void Pop_Screen(CPacket& screenPack)
	{
		m_queScreenPackets.pop_front(screenPack);
	}
	size_t Size()
	{
		return m_queScreenPackets.size();
	}
};

