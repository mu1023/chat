#include<cstdio>
#include<cstring>
#include<iostream>
#include<thread>
#include"TcpSocket.h"
#include"common_socket.h"
#define MAXMSGSIZE 1500
#define OPORT 55555
const char *ip = "127.0.0.1";
union int_c
{
	uint32_t len;
	char a[4];
};

int main()
{

	ConnSocket conn_sock;
	conn_sock.set_Socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));

	if (conn_sock.Connect(ip, OPORT, 10000) == false)
	{

		printf("Connect error\n");
		return 0;
	}
	std::thread th([&]
		{
			char message[MAXMSGSIZE], message1[MAXMSGSIZE];
			while (conn_sock.get_Socket() != -1)
			{
				// printf("?\n");
				uint32_t mtype;
				//printf("type: ");
				scanf("%u", &mtype);
				uint32_t len = 0;
				if (mtype != MESSAGEEXIT)
				{
					scanf("%s", message);
					len = strlen(message);
				}
				else len = 0;

#ifdef DEBUG 
				printf("len = %d\n", len);
#endif
				ProtocolFormat(mtype, message, message1, len);
#ifdef DEBUG 
				u_int len1 = (u_int)(*(u_int*)&message1);
				printf("len : %u %u\n", len1, ntohl(len1));
#endif
				if (conn_sock.Send(message1, len))
				{
#ifdef DEBUG
					printf("Send \n");
#endif
				}
			}

		});
	th.detach();
	fd_set fset;
	char data[1500];
	uint32_t len;
	while (conn_sock.get_Socket() != -1)
	{
		FD_ZERO(&fset);
		int maxfd = conn_sock.get_Socket();
		int fd = maxfd;
		FD_SET(fd, &fset);
		timeval tml;
		tml.tv_sec = 0;
		tml.tv_usec = 500;
		int flag = select(maxfd + 1, &fset, NULL, NULL, &tml);
		if (FD_ISSET(fd, &fset))
		{
			conn_sock.Recv();
		}
		while (conn_sock.get_one_message(data, len))
		{
#ifdef DEBUG
			printf("get message :\n");
#endif
			data[len] = '\0';
			printf("recv message: %s\n", data + 8);
		}
	}
	printf("close!\n");
}
