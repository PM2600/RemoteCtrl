#include "pch.h"
#include "ClientSocket.h"

CClientSocket* CClientSocket::m_instance = NULL;
CClientSocket::CHelper CClientSocket::m_helper;

CClientSocket* pclient = CClientSocket::getInstance(); // ???

std::string GetErrInfo(int wsaErrCode) {
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}

bool CClientSocket::InitSocket()
{
	//TRACE("nIP=%08x\r\n", nIP);
	if (m_sock != INVALID_SOCKET)
		CloseSocket();
	m_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) {
		printf("socket error\n");
		return false;
	}

	SOCKADDR_IN serv_adr;
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_port = htons(m_nPort);
	serv_adr.sin_addr.s_addr = htonl(m_nIP);
	if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
		AfxMessageBox("指定的ip地址不存在");
		return false;
	}

	int ret = connect(m_sock, (SOCKADDR*)&serv_adr, sizeof(SOCKADDR));
	if (ret == -1) {
		AfxMessageBox("连接失败");
		TRACE("连接失败: %d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
		return false;
	}
	return true;
}

bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
{
	if (m_sock == INVALID_SOCKET && m_hThread == INVALID_HANDLE_VALUE) {
		//if (InitSocket() == false) {
		//	return false;
		//}
		m_hThread = (HANDLE)_beginthread(&CClientSocket::threadEntry, 0, this);
		TRACE("start thread\r\n");
	}
	m_lock.lock();
	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>&>(pack.hEvent, lstPacks));
	m_mapAutoClosed.insert(std::pair<HANDLE, bool>(pack.hEvent, isAutoClosed));

	m_lstSend.push_back(pack);
	m_lock.unlock();
	WaitForSingleObject(pack.hEvent, INFINITE);
	std::map<HANDLE, std::list<CPacket>&>::iterator it;
	it = m_mapAck.find(pack.hEvent);
	if (it != m_mapAck.end()) {
		m_lock.lock();
		m_mapAck.erase(it);
		m_lock.unlock();
		return true;
	}
	return false;
}

//bool CClientSocket::SendPacket(const CPacket& pack, std::list<CPacket>& lstPacks, bool isAutoClosed)
//{
//	if (m_sock == INVALID_SOCKET) {
//		//if (InitSocket() == false) {
//		//	return false;
//		//}
//		_beginthread(&CClientSocket::threadEntry, 0, this);
//	}
//	auto pr = m_mapAck.insert(std::pair<HANDLE, std::list<CPacket>>(pack.hEvent, std::list<CPacket>()));
//
//	m_lstSend.push_back(pack);
//	WaitForSingleObject(pack.hEvent, INFINITE);
//	std::map<HANDLE, std::list<CPacket>>::iterator it;
//	it = m_mapAck.find(pack.hEvent);
//	if (it != m_mapAck.end()) {
//		std::list<CPacket>::iterator i;
//		for (i = it->second.begin(); i != it->second.end(); i++) {
//			lstPacks.push_back(*i);
//		}
//		m_mapAck.erase(it);
//		return true;
//	}
//	return false;
//}

void CClientSocket::threadEntry(void* arg)
{
	CClientSocket* thiz = (CClientSocket*)arg;
	thiz->threadFunc();
}

void CClientSocket::threadFunc()
{
	std::string strBuffer;
	strBuffer.resize(BUFFER_SIZE);
	char* pBuffer = (char*)strBuffer.c_str();
	int index = 0;
	InitSocket();
	while (m_sock != INVALID_SOCKET) {
		if (m_lstSend.size() > 0) {
			TRACE("lstSend size: %d\r\n", m_lstSend.size());
			m_lock.lock();
			CPacket& head = m_lstSend.front();
			m_lock.unlock();
			if (Send(head) == false) {
				TRACE("发送失败\r\n");
				continue;
			}
			std::map<HANDLE, std::list<CPacket>&>::iterator it;
			it = m_mapAck.find(head.hEvent);
			if (it != m_mapAck.end()) {
				std::map<HANDLE, bool>::iterator it0 = m_mapAutoClosed.find(head.hEvent);
				do {
					int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
					if ((length > 0) || (index > 0)) {
						index += length;
						size_t size = (size_t)index;
						CPacket pack((BYTE*)pBuffer, size);

						if (size > 0) {
							// TODO
							pack.hEvent = head.hEvent;
							it->second.push_back(pack);
							memmove(pBuffer, pBuffer + size, index - size);
							index -= size;
							if (it0->second) {
								SetEvent(head.hEvent);
								break;
							}
						}
					}
					else if (length <= 0 && index <= 0) {
						CloseSocket();
						SetEvent(head.hEvent);
						m_mapAutoClosed.erase(it0);
						break;
					}

				} while (it0->second == false);

			}
			m_lock.lock();
			m_lstSend.pop_front();
			m_lock.unlock();
			if (InitSocket() == false) {
				InitSocket();
			}
		}
		Sleep(1);
	}
	CloseSocket();
}

//void CClientSocket::threadFunc()
//{
//	std::string strBuffer;
//	strBuffer.resize(BUFFER_SIZE);
//	char* pBuffer = (char*)strBuffer.c_str();
//	int index = 0;
//	InitSocket();
//	while (m_sock != INVALID_SOCKET) {
//		if (m_lstSend.size() > 0) {
//			TRACE("lstSend size: %d\r\n", m_lstSend.size());
//			CPacket& head = m_lstSend.front();
//			if (Send(head) == false) {
//				TRACE("发送失败\r\n");
//				continue;
//			}
//			std::map<HANDLE, std::list<CPacket>>::iterator it;
//			it = m_mapAck.find(head.hEvent);
//
//			int length = recv(m_sock, pBuffer + index, BUFFER_SIZE - index, 0);
//			if (length > 0 || index > 0) {
//				index += length;
//				size_t size = (size_t)index;
//				CPacket pack((BYTE*)pBuffer, size);
//
//				if (size > 0) {
//					// TODO
//					pack.hEvent = head.hEvent;
//					it->second.push_back(pack);
//					SetEvent(head.hEvent);
//					memmove(pBuffer, pBuffer + size, index - size);
//					index -= size;
//				}
//			}
//			else if (length <= 0 && index <= 0) {
//				CloseSocket();
//				SetEvent(head.hEvent);
//			}
//
//			m_lstSend.pop_front();
//			InitSocket();
//		}
//	}
//	CloseSocket();
//}


bool CClientSocket::Send(const CPacket& pack)
{
	if (m_sock == -1)
		return false;
	std::string strOut;
	pack.Data(strOut);
	return send(m_sock, strOut.c_str(), strOut.size(), 0) > 0;
}
