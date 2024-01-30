#include "pch.h"
#include "Command.h"

CCommand::CCommand()
{
	struct {
		int nCmd;
		CMDFUNC func;
	}data[] = {
		{1, NULL},
		{2, NULL},
		{3, NULL},
		{4, NULL},
		{5, NULL},
		{6, NULL},
		{7, NULL},
		{8, NULL},
		{9, NULL},
		{1981, NULL},
		{-1, NULL}
	};
	for (int i = 0; data[i].nCmd != -1; i++) {
		m_mapFunction.insert(std::pair<int, CMDFUNC>(data[i].nCmd, data[i].func));
	}
}
