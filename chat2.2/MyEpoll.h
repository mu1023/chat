
#ifndef MyEpollh
#define MyEpollh
#include<sys/epoll.h>
#include <sys/resource.h>
#include<stdio.h>
#include<list>
#include<unordered_map>
#include<map>
#include"TcpSocket.h"
const int MAXEPOLLSIZE = 5010;
class MyEpoll
{
public:
	MyEpoll();
	~MyEpoll();
	bool Create();
	bool Add(int fd, uint32_t EventsOption);
	int Wait(int ms=500);
	bool fd_number_sub();
	int GetEventsFd(int id);
public:
	int epollfd;
	int fd_number;
	int eventNumber;
	struct epoll_event events[MAXEPOLLSIZE];
	//struct rlimit rt;
};
#endif // My