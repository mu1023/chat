
#ifndef Serverh
#define Serverh
#include<sys/epoll.h>
#include <sys/resource.h>
#include<stdio.h>
#include<list>
#include<unordered_map>
#include<queue>
#include<mutex>
#include<map>
#include"TcpSocket.h"
#include"MyEpoll.h"
const int MAXCONNSIZE = 5005;
const int MAXQUEUESIZE = 2000;
const int MSGLENGTH = 1500;
/*
class MessageQueue
{

public:

	MessageQueue()
	{
		head = 0;
		tail = 0;
	}
	char* front(uint32_t  &len, int &fd)
	{
		if (empty())return NULL;
		char *ret = Message[head];
		len = Len[head];
		fd = sock_fd[head];
		return ret;
	}
	bool push(char *data, uint32_t  len, int fd)
	{
		if (full() || len > MSGLENGTH)return false;
		Len[tail] = len;
		sock_fd[tail] = fd;
		memcpy(Message[tail], data, len);
		
		++tail;
		tail %= MAXQUEUESIZE;
		return true;
	}
	void pop()
	{
		if (empty())return;
		++head;
		head %= MAXQUEUESIZE;
		return;
	}
	int size()
	{
		return (tail + MAXQUEUESIZE - head) % MAXQUEUESIZE;
	}
	bool full()
	{
		return tail + 1 == head;
	}
	bool empty()
	{
		return tail == head;
	}
private:
	char Message[MAXQUEUESIZE][MSGLENGTH];
	uint32_t Len[MAXQUEUESIZE];
	int sock_fd[MAXQUEUESIZE];
	int head, tail;
};*/
struct MInfo
{
	char Message[MSGLENGTH];
	uint32_t Len;
	int Sock_fd;
	void init(char *str,int len,int sock_fd)
	{
		Len = len;
		Sock_fd = sock_fd;
		memcpy(Message, str, len);
	}
};

class Server
{
public:
	Server();
	int Handle(u_short port, const char *ip);
private:
	bool OpenServer(u_short port, const char *ip);
	bool Recv(int fd);
	bool SendMessage();
	bool MessageQueuePush(int fd);
	bool SendBuffer();
	void RecvBufferToMessageQueue();
	void WelcomeMember(int fd);
	int MessageAddName(char data[],const char name[]);
	std::unordered_map<int, ConnSocket>::iterator Erase(int fd);
private:
	//int events_len;
	MyEpoll ep;
	ListenSocket lis;
	std::unordered_map<int, ConnSocket>mp;
	std::unordered_map<int, std::string>mpname;
	std::unordered_map<int, int>ocount;
	std::queue<MInfo>messq;

};
#endif