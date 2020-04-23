
#include "MyEpoll.h"
#include <stdio.h>
#include<sys/epoll.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include<list>
#include<algorithm>

MyEpoll::MyEpoll() :fd_number(1)
{
	fd_number = 0;
	epollfd = -1;
}

MyEpoll::~MyEpoll()
{
	if(epollfd!=-1)close(epollfd);
}


bool MyEpoll::Create()
{
	epollfd = epoll_create(MAXEPOLLSIZE);
	if (epollfd == -1)
	{
		printf("epollfd == -1\n");
		exit(0);
	}
	return true;
}

bool MyEpoll::Add(int fd, uint32_t EventsOption)
{
	struct epoll_event ev;
	ev.events = EventsOption;
	ev.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
#ifdef DEBUG
		printf("add fd error\n");
#endif
		return false;
	}
#ifdef DEBUG
	printf("add fd %d\n", fd);
#endif
	fd_number++;
	return true;
}
int MyEpoll::Wait(int ms)
{
	eventNumber = epoll_wait(epollfd, events, fd_number, ms);
	return eventNumber;
}

bool MyEpoll::fd_number_sub()
{
	/*	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) < 0) {
	#ifdef DEBUG
			printf("fd: %d\n", fd);
			printf("epollfd delete error : %d\n", errno);
	#endif
			//return false;
		}*/
	fd_number--;
	return true;
}

int MyEpoll::GetEventsFd(int id)
{
	if (id >= eventNumber || id<0)return -1;
	return events[id].data.fd;
}


