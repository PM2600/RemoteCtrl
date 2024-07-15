#include "pch.h"
#include "Common.h"
#pragma warning(disable:4996)
void Dump(BYTE* pData, DWORD len,DWORD col)
{
	std::string str = "";
	for (int i = 0; i < len; i++)
	{
        char ch[3];
        sprintf(ch, "%02X", pData[i] & 0xFF);
        str += ch;
        str += " ";
		if ((i != 0) && (i % (col - 1) == 0)) str += "\n";
	}
	TRACE("\r\n----------Dump Address : %X Length : %d Column : %d----------\r\n%s\r\n", pData, len, col, str.c_str());
}

std::string GetErrInfo(int wsaErrCode)
{
    std::string ret;
    LPVOID lpMsgBuf = NULL;
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL,
        wsaErrCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    ret = (char*)lpMsgBuf;
    LocalFree(lpMsgBuf);
    return ret;
}

void Dump(const CPacket& pack,int col)
{
    std::string str = "";
    BYTE* pData = (BYTE*) &pack;
    for (int i = 0; i < (sizeof(pack.nHead) + sizeof(pack.nLength) + pack.nLength); i++)
    {
        char ch[3];
        sprintf(ch, "%02X", pData[i] & 0xFF);
        str += ch;
        str += " ";
        if (((i + 1) % col) == 0) str += "\r\n";
    }
    char packInfo[256]{};
    sprintf(packInfo, "%s+ Packet Length(Total): %d\r\n", packInfo, (sizeof(pack.nHead) + sizeof(pack.nLength) + pack.nLength));
    sprintf(packInfo, "%s+ Packet Length(Part ): %d\r\n", packInfo, pack.nLength);
    sprintf(packInfo, "%s+ Packet Command      : %d\r\n", packInfo, pack.nCmd);
    sprintf(packInfo, "%s+ Packet Sum Check    : %d\r\n",     packInfo, pack.nSum);
    sprintf(packInfo, "%s+ Packet Data         :       ", packInfo);
    char tips[]{ "Dump CPacket" };
    int a = col * 2 + (col - 1);
    int b = (a - strlen(tips)) / 2;
    char left_[256]{};
    char right_[256]{};
    char end_[256] {};
    for (int i = 0; i < b; i++)
    {
        left_[i] = '-';
        right_[i] = '-';
        if (i == b - 1)
        {
            if (b * 2 + strlen(tips) < a) right_[i + 1] = '-';
        }
    }
    for (int i = 0; i < a; i++) end_[i]  = '-';
    TRACE("\r\n%s%s%s\r\n%s\r\n%s\r\n%s\r\n", left_, tips, right_,packInfo,str.c_str(),end_);

}