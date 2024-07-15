#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket::m_instance = nullptr;
CServerSocket::CLifeManager CServerSocket::m_life;