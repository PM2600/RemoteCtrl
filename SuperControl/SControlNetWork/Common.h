#pragma once
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <string>
#include <string.h>

#pragma pack(push)
#pragma pack(1)

#define _IN_OUT_


class CPacket
{
public:
	unsigned short			nHead;
	unsigned int 			nLength;
	unsigned short			nCmd;
	std::string				sData;
	unsigned short			nSum;
private:
	std::string				sOut;

public:
	CPacket() {}
	/// <summary>
	/// 构建数据包
	/// </summary>
	/// <param name="_nCmd		">命令</param>
	/// <param name="_bData		">数据</param>
	/// <param name="_nLength	">数据长度</param>
	CPacket(unsigned short _nCmd, unsigned char* _bData = NULL, unsigned int _nLength = 0)
	{
		nHead = 0xFEFF;
		nLength = (uint32_t)(sizeof(nCmd) + _nLength + sizeof(nSum));
		nCmd = _nCmd;
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum)); 
		memcpy((void*)sData.c_str(), _bData, _nLength);
		nSum = 0;
		
		for (uint16_t i = 0; i < _nLength; i++) 
			nSum += (uint16_t)(sData[i]&0xFF);
	}
	CPacket(unsigned char* bParserAddr, _IN_OUT_ int& len)
	{
		if (len == 0) return;
		//长度小于最小整个包长
		if (int(sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len)
		{
			len = 0; return;
		}
		//找包头
		size_t i = 0;
		for (; (int)i < len - 1; i++) if (*(unsigned short*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//找到了包头，按照最小的整个包长来算，也超出了nLength
		if (int(i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1)) > len) 
		{ len = 0; return; };
		//指向包长
		i += sizeof(nHead);
		//解析包长
		nLength = *(unsigned int*) & bParserAddr[i];
		//找到了包头，按照整个包长来算，也超出了nLength
		if (int(i + (sizeof(nLength) + nLength - 1)) > len) 
		{ len = 0; return; };
		//指向命令
		i += sizeof(nLength);
		//解析命令
		nCmd = *(unsigned short*)&bParserAddr[i];
		//指向数据
		i += sizeof(nCmd);
		//解析数据
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//指向和校验
		i += sData.size();
		//解析和校验
		nSum = *(unsigned short*)&bParserAddr[i];
		//指向和校验的下一位
		i += sizeof(nSum);
		//和校验
		if (nCmd == 5)
		{
			len = (int)i;
		}
		else
		{
			unsigned short tSum = 0;
			for (size_t j = 0; j < sData.size(); j++) 
				tSum += (uint16_t)(sData[j] & 0xFF);
			if (tSum == nSum) len = (int)i;
		}
	}
	unsigned char* Data()
	{
		sOut.resize(sizeof(nHead) + sizeof(nLength) + nLength);
		unsigned char* pData = (unsigned char*)sOut.c_str();
		size_t i = 0;
		*(unsigned short*)&pData[i] = nHead; i += sizeof(nHead);
		*(unsigned int*)& pData[i] = nLength; i += sizeof(nLength);
		*(unsigned short*)&pData[i] = nCmd; i += sizeof(nCmd);
		memcpy(&pData[i], sData.c_str(), sData.size()); i += sData.size();
		*(unsigned short*)&pData[i] = nSum; i += sizeof(nSum);
		return pData;
	}
	uint16_t Size()
	{
		return (uint16_t)(sizeof(nHead) + sizeof(nLength) + nLength);
	}
	CPacket& operator=(const CPacket& _pack)
	{
		nHead = _pack.nHead;
		nLength = _pack.nLength;
		nCmd = _pack.nCmd;
		sData.resize(_pack.sData.size());
		memcpy((void*)sData.c_str(), _pack.sData.c_str(), _pack.sData.length());
		nSum = _pack.nSum;
		return *this;
	}
	std::string ToString()
	{
		std::string info;
		char chInfo[0xff]{};
		sprintf(chInfo, "%snHead    : %d\r\n", chInfo, nHead);
		sprintf(chInfo, "%snLength  : %d\r\n", chInfo, nLength);
		sprintf(chInfo, "%ssDataLen : %d\r\n", chInfo, sData.size());
		sprintf(chInfo, "%snCmd     : %d\r\n", chInfo, nCmd);
		sprintf(chInfo, "%snSum     : %d\r\n", chInfo, nSum);
		info = chInfo;
		return info;
	}
	std::string ToString2()
	{
		std::string info;
		char chInfo[0xff]{};
		sprintf(chInfo, "%snHead : %d ", chInfo, nHead);
		sprintf(chInfo, "%snLength : %d ", chInfo, nLength);
		sprintf(chInfo, "%ssDataLen : %d ", chInfo, sData.size());
		sprintf(chInfo, "%snCmd : %d ", chInfo, nCmd);
		sprintf(chInfo, "%snSum : %d ", chInfo, nSum);
		info = chInfo;
		return info;
	}
};

struct MUserInfo
{
	int					tcpSock;
	unsigned long long  id;
	char				ip[16];
	short				port;
	long long			last;

	MUserInfo(const char* _ip, const short _port)
	{
		//获得当前时间错（毫秒级别）
		/*struct timeval tv;
		gettimeofday(&tv, NULL);
		id = tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
		memcpy(ip, _ip, sizeof(ip));
		port = _port;
	}
};

struct ConnectIds
{
	unsigned long long  id0;
	unsigned long long  id1;
};

#pragma pack(pop)

