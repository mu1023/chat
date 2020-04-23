
#include <iostream>
#include <algorithm>
#include<list>
#include "TcpSocket.h"
#include "MyEpoll.h"
#include "Server.h"
#define OPORT 55555
const char *ip = "127.0.0.1";
const int FD_SIZE = 1024;

int main()
{
	Server serv;
	serv.Handle(OPORT, ip);
	return 0;
}
