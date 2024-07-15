#include <cstdio>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <memory>
#include <vector>
#include "UDPPassNetWork.h"

int main()
{
	UDPPassNetWork net_work("192.168.88.149", 16888, 18888);
	net_work.Invoke();
	
	printf("input any key done...\n");
	getchar();

	return 0;
}