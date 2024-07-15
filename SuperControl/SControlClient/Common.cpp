#include "pch.h"
#include "Common.h"

void Dump(BYTE* pData, DWORD len,DWORD col)
{
	std::string str = "";
	for (DWORD i = 0; i < len; i++)
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