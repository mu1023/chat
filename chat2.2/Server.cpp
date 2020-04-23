
#include"Server.h"
#include "common_socket.h"
bool Server::OpenServer(u_short port, const char *ip)
{
	if (!lis.open_server(port, ip))
	{
		printf("open fail\n");
		return false;
	}
#ifdef DEBUG1
	printf("enter openserver1\n");
#endif
	if (!ep.Add(lis.get_Socket(), EPOLLIN | EPOLLERR))
	{
		return false;
	}
	printf("open success\n");
	return true;
}

bool Server::Recv(int fd)
{
	if (mp.find(fd) == mp.end())return false;
	return mp[fd].Recv();
}
std::unordered_map<int, ConnSocket>::iterator Server::Erase(int fd)
{
	std::unordered_map<int, ConnSocket>::iterator it = mp.end();
	if (mp.find(fd) != mp.end())
	{
		mp[fd].close_socket();
		it = mp.erase(mp.find(fd));
	}
	if (mpname.find(fd) != mpname.end())
	{
		mpname.erase(mpname.find(fd));
	}
	if (ocount.find(fd) != ocount.end())
	{
		ocount.erase(ocount.find(fd));
	}
	ep.fd_number_sub();
	return it;
}
bool Server::SendMessage()
{
	
	MyEpoll write_ep;
	write_ep.Create();
	int onum = 0; //多少个连接的缓冲区即将满
	while (!messq.empty())
	{
		MInfo data = messq.front();
		messq.pop();
		enum MESSAGETYPE mt = getMessageType(data.Message);
		switch (mt)
		{
		case MESSAGEBROADCAST: {
#ifdef DEBUG
			printf("-----------------------------data :   %d\n", data.Len);
#endif
			for (auto it = mp.begin(); it != mp.end();)
			{
				//
				it->second.Send(data.Message, data.Len);
					int cfd = it->first;
				   if(it->second.occupancy()>0.99){
					   
					  
					   ocount[cfd]++;
					   printf("%s 缓冲区溢出警告第%d次\n",mpname[cfd].c_str(),ocount[cfd]);
					   if (ocount[cfd] > 3)
					   {
						   it = Erase(it->first);
					   }
					   else
					   {
						   onum++;
						   write_ep.Add(it->second.get_Socket(), EPOLLOUT);
						   it++;
					   }
					   
				   }
				   else
				   {
					   ocount[cfd] = 0;
					   it++;
				   }
			
			}
		}
							   break;
		case MESSAGEECHO: {
			if (mp.find(data.Sock_fd) != mp.end())
			{
				mp[data.Sock_fd].Send(data.Message, data.Len);
				if (mp[data.Sock_fd].occupancy() > 0.99)
				{
					
					ocount[data.Sock_fd]++;
					printf("%s 缓冲区溢出警告第%d次\n", mpname[data.Sock_fd].c_str(), ocount[data.Sock_fd]);
					if (ocount[data.Sock_fd] > 3)
					{
						Erase(data.Sock_fd);
					}
					else
					{
						
						onum++;
						write_ep.Add(data.Sock_fd, EPOLLOUT);
					}
					
				}
			}
			else
			{
				ocount[data.Sock_fd] = 0;
			}
		}
						  break;
		case MESSAGEEXIT: {
			if (mp.find(data.Sock_fd) != mp.end())
			{
				//mp[data.Sock_fd].close_socket();
				Erase(data.Sock_fd);
			}
		}
						  break;
		case MESSAGEERROR:
			break;
		default:
			break;
		}
		
		if (onum > 0)break;
	}
	if (onum == 0)return true;
	int cnt = 2;
	while (cnt--)
	{
		int n = write_ep.Wait(500);
		for (int i = 0; i < n; i++)
		{
			int fd = write_ep.GetEventsFd(i);
			if (mp.find(fd) != mp.end())
			{
				mp[fd].Send_Buffer();
			}
		}
	}
	
	return true;
}
int md = 0;
bool Server::MessageQueuePush(int fd)
{
	if (mp.find(fd) == mp.end())
	{
		return false;
	}
	char data[MSGLENGTH];
	uint32_t len = 0;//xx
	ConnSocket *conn = &mp[fd];

	while (messq.size() < MAXQUEUESIZE && conn->get_one_message(data, len))
	{
#ifdef DEBUG
		printf("push data : %s len:%d\n", data, len);
#endif
		printf("%d %s 发送了一条消息, 长度 =  %d\n", ++md,mpname[fd].c_str(), len);
		char name[MSGLENGTH];
		strcpy(name,mpname[fd].c_str());
		len += MessageAddName(data,name);
	//	printf("len =%d\n",len);
		MInfo me;
		me.init(data,len,fd);
		messq.push(me);
	}

	return true;
}

bool Server::SendBuffer()
{
	for (auto it = mp.begin(); it != mp.end(); it++)
	{
		it->second.Send_Buffer();
	}
	return false;
}
int del = 0;
void Server::RecvBufferToMessageQueue()
{
	for (auto it = mp.begin(); it != mp.end(); )
	{
		if (!it->second.is_open())
		{
		/*	if(ocount.find(it->first)!= ocount.end())ocount.erase(ocount.find(it->first));
		 	if(mpname.find(it->first) != mpname.end())mpname.erase(mpname.find(it->first));
			it = mp.erase(it);*/
			it = Erase(it->first);
		//	ep.fd_number_sub();
		//	printf("del %d\n",del++);
		}
		else if(messq.size() < MAXQUEUESIZE)
		{
			MessageQueuePush(it->second.get_Socket());
			++it;
		}
		else ++it;

	}
}

Server::Server()
{
}

void Server::WelcomeMember(int fd)
{
	if (fd < 0 || messq.size()>MAXQUEUESIZE)return;
	mpname[fd] = RandomName();
	//char wl[] = ",欢迎你!";
	int tp = MESSAGEECHO;
	printf("%s 加入聊天室\n",mpname[fd].c_str());
	char ms1[MSGLENGTH] = "";
	char ms2[MSGLENGTH] = "";
	strcpy(ms1, (mpname[fd]+",欢迎你").c_str());
	uint32_t len = strlen(ms1);
	ProtocolFormat(tp, ms1, ms2, len);
	MInfo me;
	me.init(ms2, len, fd);
	if(messq.size()<MAXQUEUESIZE)messq.push(me);
}
int Server::MessageAddName(char data[],const char name[])
{
	if (data == NULL || name == NULL)return 0;

	uint32_t len = (uint32_t)ntohl((uint32_t)(*(uint32_t *)data));
	char n_str[MSGLENGTH];
	uint32_t n_len = strlen(name); 
	
	memcpy(n_str, name, n_len);
	uint32_t  l= strlen("说:");
	memcpy(n_str+n_len, "说:", l);
	//n_str[n_len] = ':';

	n_len = n_len + l;	
	//printf("%dxxxx\n", n_len);
	memmove(data+PROTOCOLHEADLENGTH+n_len,data+PROTOCOLHEADLENGTH,len);
	memmove(data+PROTOCOLHEADLENGTH, n_str,n_len);
	len += n_len;
	len = (uint32_t)htonl(len);
	memmove(data, (char*)&len, 4);
	return n_len;
}
int Server::Handle(u_short port, const char *ip)
{
	if (ep.Create() == false || OpenServer(port, ip) == false)
	{
		printf("Server open error\n");
		return 0;
	}
	int cnt = 0;
	while (true)
	{
		printf("%d 个客户\n",ep.fd_number-1);
		int n = 0;
		n = ep.Wait();
		for (int i = 0; i < n; i++)
		{
			if (ep.GetEventsFd(i) == lis.get_Socket())
			{
#ifdef DEBUG
				printf("add client\n");
#endif
				int connfd = 0;//xx
				while ((connfd = accept(lis.get_Socket(), NULL, NULL)) >= 0)
				{
					//printf("%d: add fd %d\n", ++cnt, connfd);
					ep.Add(connfd, EPOLLIN | EPOLLERR);
					mp.insert(std::make_pair(connfd, ConnSocket(connfd)));
					WelcomeMember(connfd);
				}
			}
			else if ( messq.size() < MAXQUEUESIZE && mp.find(ep.GetEventsFd(i)) != mp.end())
			{
				Recv(ep.GetEventsFd(i));
				//printf("wtfk\n");
				MessageQueuePush(ep.GetEventsFd(i));
			}
		}
		SendMessage();
		SendBuffer();
		RecvBufferToMessageQueue();
	}
	return 1;
}

