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
		printf("�ͻ����׽��ִ���ʧ��\r\n");
	}

	m_iocpServer->IocpBindSocket(m_clntSocket);
}

void Screen2(CPacket& pack)
{

    printf("start: %lld\r\n", GetTickCount64());

    CDC* pDC;//��ĻDC
    pDC = CDC::FromHandle(GetDC(NULL));//��ȡ��ǰ������ĻDC
    int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);//�����ɫģʽ
    int Width = pDC->GetDeviceCaps(HORZRES);
    int Height = pDC->GetDeviceCaps(VERTRES);
    
    CDC memDC;//�ڴ�DC
    memDC.CreateCompatibleDC(pDC);

    CBitmap memBitmap, * oldmemBitmap;//��������Ļ���ݵ�bitmap
    memBitmap.CreateCompatibleBitmap(pDC, Width, Height);

    oldmemBitmap = memDC.SelectObject(&memBitmap);//��memBitmapѡ���ڴ�DC
    memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY);//������Ļͼ���ڴ�DC

    //���´��뱣��memDC�е�λͼ���ļ�
    BITMAP bmp;
    memBitmap.GetBitmap(&bmp);//���λͼ��Ϣ

    BITMAPINFOHEADER bih = { 0 };//λͼ��Ϣͷ
    bih.biBitCount = bmp.bmBitsPixel;//ÿ�������ֽڴ�С
    bih.biCompression = BI_RGB;
    bih.biHeight = bmp.bmHeight;//�߶�
    bih.biPlanes = 1;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;//ͼ�����ݴ�С
    bih.biWidth = bmp.bmWidth;//���

    BITMAPFILEHEADER bfh = { 0 };//λͼ�ļ�ͷ
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);//��λͼ���ݵ�ƫ����
    bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;//�ļ��ܵĴ�С
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
        (LPBITMAPINFO)&bih, DIB_RGB_COLORS);//��ȡλͼ����
    memDC.SelectObject(oldmemBitmap);

}
