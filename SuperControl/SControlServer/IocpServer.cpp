#include "pch.h"
#include "IocpServer.h"

CCmdProcessor cmdProc;
CMQueue<CPacket> lstScreenPcks;
std::mutex mtx_;

CMClient::CMClient(CIocpServer* serv, HANDLE iocp) :
	m_accept(new ACCEPTOVERLAPPED()),
	m_recv(new RECVOVERLAPPED()),
	m_send(new SENDOVERLAPPED()),
	m_close(new CLOSEOVERLAPPED())
{
	m_accept->m_client = this;
	m_recv->m_client = this;
	m_send->m_client = this;
	m_close->m_client = this;
	m_iocpServer = serv;
	m_iocp = iocp;
	m_tick = GetTickCount64();

	m_clntSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_clntSocket == INVALID_SOCKET)
	{
		printf("客户端套接字创建失败\r\n");
	}

	m_iocpServer->IocpBindSocket(m_clntSocket);
}

void Screen2(CPacket& pack)
{

    printf("start: %lld\r\n", GetTickCount64());

    CDC* pDC;//屏幕DC
    pDC = CDC::FromHandle(GetDC(NULL));//获取当前整个屏幕DC
    int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//获得颜色模式
    int Width = pDC->GetDeviceCaps(HORZRES);
    int Height = pDC->GetDeviceCaps(VERTRES);
    
    CDC memDC;//内存DC
    memDC.CreateCompatibleDC(pDC);

    CBitmap memBitmap, * oldmemBitmap;//建立和屏幕兼容的bitmap
    memBitmap.CreateCompatibleBitmap(pDC, Width, Height);

    oldmemBitmap = memDC.SelectObject(&memBitmap);//将memBitmap选入内存DC
    memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//复制屏幕图像到内存DC

    //以下代码保存memDC中的位图到文件
    BITMAP bmp;
    memBitmap.GetBitmap(&bmp);//获得位图信息

    BITMAPINFOHEADER bih = { 0 };//位图信息头
    bih.biBitCount = bmp.bmBitsPixel;//每个像素字节大小
    bih.biCompression = BI_RGB;
    bih.biHeight = bmp.bmHeight;//高度
    bih.biPlanes = 1;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//图像数据大小
    bih.biWidth = bmp.bmWidth;//宽度

    BITMAPFILEHEADER bfh = { 0 };//位图文件头
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//到位图数据的偏移量
    bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//文件总的大小
    bfh.bfType = (WORD)0x4d42;

    int size = bmp.bmWidthBytes * bmp.bmHeight + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    UINT pos = 0;

    pack.sData.resize(size);
    pack.nCmd = 5;
    pack.nHead = 0xFEFF;
    pack.nLength = sizeof(pack.nCmd) + size + sizeof(pack.nSum);
    //pack.nSum = 0; for (size_t i = 0; i < size; i++) pack.nSum += ((BYTE)pack.sData[i]) & 0xFF;
    byte* p = (byte*)pack.sData.c_str();

    memcpy(p + pos, &bfh, sizeof(BITMAPFILEHEADER)); pos += sizeof(BITMAPFILEHEADER);
    memcpy(p + pos, &bih, sizeof(BITMAPINFOHEADER)); pos += sizeof(BITMAPINFOHEADER);
    GetDIBits(memDC.m_hDC, (HBITMAP)memBitmap.m_hObject, 0, Height, p + pos,
        (LPBITMAPINFO)&bih, DIB_RGB_COLORS);//获取位图数据
    memDC.SelectObject(oldmemBitmap);

}
