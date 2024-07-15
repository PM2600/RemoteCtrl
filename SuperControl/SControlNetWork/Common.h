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
	/// �������ݰ�
	/// </summary>
	/// <param name="_nCmd		">����</param>
	/// <param name="_bData		">����</param>
	/// <param name="_nLength	">���ݳ���</param>
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
		//����С����С��������
		if (int(sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum)) > len)
		{
			len = 0; return;
		}
		//�Ұ�ͷ
		size_t i = 0;
		for (; (int)i < len - 1; i++) if (*(unsigned short*)&bParserAddr[i] == 0xFEFF) { nHead = 0xFEFF; break; };
		//�ҵ��˰�ͷ��������С�������������㣬Ҳ������nLength
		if (int(i + (sizeof(nHead) + sizeof(nLength) + sizeof(nCmd) + 0 + sizeof(nSum) - 1)) > len) 
		{ len = 0; return; };
		//ָ�����
		i += sizeof(nHead);
		//��������
		nLength = *(unsigned int*) & bParserAddr[i];
		//�ҵ��˰�ͷ�����������������㣬Ҳ������nLength
		if (int(i + (sizeof(nLength) + nLength - 1)) > len) 
		{ len = 0; return; };
		//ָ������
		i += sizeof(nLength);
		//��������
		nCmd = *(unsigned short*)&bParserAddr[i];
		//ָ������
		i += sizeof(nCmd);
		//��������
		sData.resize(nLength - sizeof(nCmd) - sizeof(nSum));
		memcpy((void*)sData.c_str(), &bParserAddr[i], sData.size());
		//ָ���У��
		i += sData.size();
		//������У��
		nSum = *(unsigned short*)&bParserAddr[i];
		//ָ���У�����һλ
		i += sizeof(nSum);
		//��У��
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
		//��õ�ǰʱ������뼶��
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

