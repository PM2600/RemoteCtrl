#pragma once
#pragma warning(disable:4267)
#pragma pack(push)
#pragma pack(1)

#define _IN_OUT_

#include <io.h>

class CPacket
{
public:
	WORD			nHead;
	DWORD			nLength;
	WORD			nCmd;
	std::string		sData;
	WORD			nSum;
private:
	std::string		sOut;
	bool			isCheck;

public:
	CPacket(){}
	/// <summary>
	/// 构建数据包
	/// </summary>
	/// <param name="_nCmd		">命令</param>
	/// <param name="_bData		">数据</param>
	/// <param name="_nLength	">数据长度</param>
	CPacket(WORD _nCmd, BYTE* _bData = NULL, DWORD _nLength = 0, bool _isCheck = true)
	{
		nHead = 0xFEFF;
		nLength = sizeof(nCmd) + _nLength + sizeof(nSum);
		nCmd = _nCmd;
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum)); memcpy((void*)sData.c_str(), _bData, _nLength);
		nSum = 0; for (size_t i = 0; i < _nLength; i++) nSum += ((BYTE)sData[i]) & 0xFF;
		isCheck = _isCheck;
	}
	CPacket(BYTE* bParserAddr, _IN_OUT_ DWORD& len)
	{
		//长度小于最小整个包长
		if ((sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len) { len = 0; return; };
		//找包头
		size_t i = 0;
		for (; i < len - 1; i++) if (*(WORD*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//找到了包头，按照最小的整个包长来算，也超出了nLength
		if ((i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1) > len)) { len = 0; return; };
		//指向包长
		i += sizeof(nHead);
		//解析包长
		nLength = *(DWORD*)&bParserAddr[i];
		//找到了包头，按照整个包长来算，也超出了nLength
		if ((i + (sizeof(nLength) + nLength - 1) > len)) { len = 0; return; };
		//指向命令
		i += sizeof(nLength);
		//解析命令
		nCmd = *(WORD*)&bParserAddr[i];
		//指向数据
		i += sizeof(nCmd);
		//解析数据
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//指向和校验
		i += sData.size();
		//解析和校验
		nSum = *(WORD*)&bParserAddr[i];
		//指向和校验的下一位
		i += sizeof(nSum);
		//和校验
		//是否进行和校验
		if (!isCheck) { len = i; return; }
		WORD tSum = 0;
		for (size_t j = 0; j < sData.size(); j++) tSum += ((BYTE)sData[j]) & 0xFF;
		if ((tSum == nSum)) len = i;
	}
	BYTE* Data()
	{
		sOut.resize(sizeof(nHead) + sizeof(nLength) + nLength);
		BYTE* pData = (BYTE*) sOut.c_str();
		int i = 0;
		*(WORD*)&pData[i] = nHead; i += sizeof(nHead);
		*(DWORD*)&pData[i] = nLength; i += sizeof(nLength);
		*(WORD*)&pData[i] = nCmd; i += sizeof(nCmd);
		memcpy(&pData[i], sData.c_str(), sData.size()); i += sData.size();
		*(WORD*)&pData[i] = nSum; i += sizeof(nSum);
		return pData;
	}
	DWORD Size()
	{
		return sizeof(nHead) + sizeof(nLength) + nLength;
	}
	CPacket(const CPacket& _pack)
	{
		nHead = _pack.nHead;
		nLength = _pack.nLength;
		nCmd = _pack.nCmd;
		sData.resize(_pack.sData.size());
		memcpy((void*)sData.c_str(), _pack.sData.c_str(), _pack.sData.length());
		nSum = _pack.nSum;
	}
	CPacket& operator=(const CPacket& _pack)
	{
		if (this != &_pack)
		{
			nHead = _pack.nHead;
			nLength = _pack.nLength;
			nCmd = _pack.nCmd;
			sData.resize(_pack.sData.size());
			memcpy((void*)sData.c_str(), _pack.sData.c_str(), _pack.sData.length());
			nSum = _pack.nSum;
		}
		return *this;
	}
};

typedef struct drive_info
{
	char drive[26];
	char drive_count;
	drive_info()
	{
		memset(drive, 0, 26);
		drive_count = 0;
	}
}DRIVEINFO,*PDRIVEINFO;

typedef struct file_info
{
	_finddata64i32_t data;
	BOOL isNull;
}FILEINFO,*PFILEINFO;

enum MOUSEBTN
{
	LEFT		= 0x1		,
	MID			= 0x2		,
	RIGHT		= 0x4		,
	NOTHING		= 0x8		,
};

enum MOUSEEVE
{
	CLICK		= 0x10		,
	DBCLICK		= 0x20		,
	DOWN		= 0x40		,
	UP			= 0x80		,
	MOVE		= 0x100		,
};

typedef struct mouse_info
{
	CPoint		ptXY;
	MOUSEBTN	nButton;
	MOUSEEVE	nEvent;
}MOUSEINFO,PMOUSEINFO;

struct MUserInfo
{
	int					tcpSock;
	unsigned long long  id;
	char				ip[16];
	short				port;
	long long			last;

	MUserInfo()
	{
		id = GetTickCount64();
	}

	MUserInfo(const char* _ip, const short _port)
	{
		memcpy(ip, _ip, sizeof(ip));
		port = _port;
	}
};


#pragma pack(pop)

void Dump(BYTE* pData, DWORD len, DWORD col = 16);
void Dump(const CPacket& pack,int col = 16);

std::string GetErrInfo(int wsaErrCode);
