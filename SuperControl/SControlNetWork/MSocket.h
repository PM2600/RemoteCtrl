#pragma once

class MSocket
{
public:
	enum TYPE
	{
		TYPE_TCP,
		TYPE_UDP,
	};
private:
	TYPE m_type;
	MSocket(TYPE ntype);
	MSocket();
	~MSocket();
};

