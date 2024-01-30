#pragma once
#include <map>
class CCommand
{
public:
	CCommand();
	~CCommand();
	int ExcuteCommand(int nCmd);
protected:
	typedef int(CCommand::* CMDFUNC)(); //成员函数指针
};

