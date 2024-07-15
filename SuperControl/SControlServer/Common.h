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
	/// �������ݰ�
	/// </summary>
	/// <param name="_nCmd		">����</param>
	/// <param name="_bData		">����</param>
	/// <param name="_nLength	">���ݳ���</param>
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
		//����С����С��������
		if ((sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len) { len = 0; return; };
		//�Ұ�ͷ
		size_t i = 0;
		for (; i < len - 1; i++) if (*(WORD*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//�ҵ��˰�ͷ��������С�������������㣬Ҳ������nLength
		if ((i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1) > len)) { len = 0; return; };
		//ָ�����
		i += sizeof(nHead);
		//��������
		nLength = *(DWORD*)&bParserAddr[i];
		//�ҵ��˰�ͷ�����������������㣬Ҳ������nLength
		if ((i + (sizeof(nLength) + nLength - 1) > len)) { len = 0; return; };
		//ָ������
		i += sizeof(nLength);
		//��������
		nCmd = *(WORD*)&bParserAddr[i];
		//ָ������
		i += sizeof(nCmd);
		//��������
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//ָ���У��
		i += sData.size();
		//������У��
		nSum = *(WORD*)&bParserAddr[i];
		//ָ���У�����һλ
		i += sizeof(nSum);
		//��У��
		//�Ƿ���к�У��
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
