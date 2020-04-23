#include "common_socket.h"
#include "TcpSocket.h"
#include <unistd.h>
Socket::Socket(int fd)
{
	socket_fd = fd;
	set_nonblock(socket_fd);
}
Socket::Socket() : socket_fd(-1)
{
}
Socket::~Socket()
{
	//if (socket_fd != -1)close_socket();
}
int Socket::get_Socket()
{
	return socket_fd;
}
bool Socket::is_open()
{
	return socket_fd != -1;
}
void Socket::set_Socket(int fd)
{
	close_socket(); //???
	socket_fd = fd;
	set_nonblock(socket_fd);
}

void Socket::close_socket()
{
	if (socket_fd == -1)
		return;
	close(socket_fd);
	printf("errno %s\n",strerror(errno));
#ifdef DEBUG
	printf("close %d\n",socket_fd);
#endif
	//close_socket();
	socket_fd = -1;
}

ListenSocket::ListenSocket()
{

}
ListenSocket::ListenSocket(int fd) : Socket(fd)
{
}

bool ListenSocket::open_server(u_short port, const char *ip)
{
	if (socket_fd >= 0)
		close_socket();
	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd == -1)
	{

		return false;
	}
	int flag = 1;
#ifdef DEBUG
	printf("opt \n");
#endif
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, (int)sizeof(flag)) != 0)//允许重用本地端口
	{
#ifdef DEBUG
		printf("opterr\n");
#endif
		close_socket();
		return false;
	}
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	if (ip == NULL)
	{
		sock_addr.sin_addr.s_addr = inet_addr(ip);
	}
	else
	{
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	sock_addr.sin_port = (u_short)htons(port);
#ifdef DEBUG
	printf("bind\n");
#endif
	if (bind(socket_fd, (const sockaddr *)&sock_addr, sizeof(sockaddr)) != 0)
	{
#ifdef DEBUG
		printf("binderr\n");
#endif
		close_socket();
#ifdef DEBUG
		printf("binderr1\n");
#endif
		return false;
	}
	set_nonblock(socket_fd);
	if (listen(socket_fd, 1023) != 0)//????
	{
		close_socket();
		return false;
	}
	return true;
}

ConnSocket::ConnSocket()
{
	recv_head = 0;
	recv_tail = 0;
	send_head = 0;
	send_tail = 0;
}

ConnSocket::ConnSocket(int fd) : Socket(fd)
{
	recv_head = 0;
	recv_tail = 0;
	send_head = 0;
	send_tail = 0;
}

bool ConnSocket::Connect(const char *ip, u_short port, int ms)
{
	if (socket_fd == -1)
	{
		perror("socket == -1");
		return false;
	}
	if (ip == NULL)
	{
		perror("ip NULL");
		return false;
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	int n = -1;
	int ret = 1;
	if ((n = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
	{
		if (errno != EINPROGRESS)
		{
			perror("connect error!1");
			ret = 0;
		}
		else
		{

			timeval tml;
			tml.tv_sec = ms / 1000;
			tml.tv_usec = ms % 1000;
			fd_set rset, wset;
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			FD_SET(socket_fd, &rset);
			FD_SET(socket_fd, &wset);
			if ((n = select(socket_fd + 1, &rset, &wset, NULL, &tml)) < 0)
			{
				perror(" connect time out");
				ret = 0;
			}
			else
			{
				int error = 0;
				socklen_t len;
				if (FD_ISSET(socket_fd, &rset) || FD_ISSET(socket_fd, &wset))
				{
					len = sizeof(error);
					if (getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
					{
						perror("connect error!2");
						ret = 0;
					}
					else ret = 1;
				}
				else ret = 0;
				if (error)
				{
					perror("connect error!3");
					ret = 0;
				}
			}
		}
	}
	else
	{
		ret = 1;
	}
	if (ret == 0)
	{
		close_socket();
		return false;
	}
	set_nonblock(socket_fd);
	return true;
}
void ConnSocket::set_Socket(int fd)
{
	Socket::set_Socket(fd);
	recv_head = 0;
	recv_tail = 0;
	send_head = 0;
	send_tail = 0;
}
bool  ConnSocket::Send_Buffer()
{
	if (socket_fd == -1)return false;
	int bufflen = send_tail - send_head;
	if (bufflen == 0)return false;
	while (bufflen > 0)
	{
		int slen = send(socket_fd, &send_buffer[send_head], (size_t)bufflen, 0);
		if (slen > 0)
		{
			bufflen -= slen;
			send_head += slen;
			if (bufflen == 0)
			{
				send_head = 0;
				send_tail = 0;
			}
		}
		if (slen == 0 || slen < 0 && errno != EAGAIN)
		{
			close_socket();
			return false;
		}
		if (slen < 0)
		{
			return true;
		}
	}
	return true;
}
bool ConnSocket::Send(char *data, int Size)
{
	if (socket_fd == -1)return false;
	if (data == NULL || Size < 0)
	{
		perror("data is NULL or size < 0");
	}
	int bufflen = send_tail - send_head;

	while (bufflen > 0)
	{
		int slen = send(socket_fd, &send_buffer[send_head], (size_t)bufflen, 0);
		if (slen > 0)
		{
			bufflen -= slen;
			send_head += slen;
		}
		if (slen < 0 && errno != EAGAIN)
		{
			close_socket();
			return false;
		}
		if (slen < 0)
		{
			if (send_buffer_size - send_tail >= Size)
			{
				memcpy(&send_buffer[send_tail], data, (size_t)Size);
				send_tail += Size;
			}
			else if (send_buffer_size - send_tail + send_head >= Size)
			{
				/*printf("%d %d \n ", send_head, send_tail);
				assert(0);*/
				memmove(&send_buffer[0], &send_buffer[send_head], (size_t)bufflen);
				send_head = 0;
				send_tail = bufflen;
				memcpy(&send_buffer[send_tail], data, (size_t)Size);
				send_tail += Size;
			}
			else
			{
				printf("%d %d %d \n ",socket_fd, send_head, send_tail);
				/*assert(0);*/
				close_socket();
				return false;
				/*sleep(1);
				continue;*/
			}
			
			return true;
		}
	}
	if (send_head == send_tail)
	{
		//	assert(0);
		send_head = 0;
		send_tail = 0;
	}
	bufflen = Size;
	int data_head = 0;
	while (bufflen > 0)
	{
		int slen = send(socket_fd, &data[data_head], (size_t)bufflen, 0);
#ifdef DEBUG
		printf("datahead %d bufflen %d %c -------\n", data_head, bufflen, data[data_head + 4]);
#endif // DEBUG
		if (slen > 0)
		{
			bufflen -= slen;
			data_head += slen;
		}
#ifdef DEBUG
		printf("send len %d\n", slen);
#endif // DEBUG
		if (slen < 0 && errno != EAGAIN)
		{
#ifdef DEBUG
			printf("socket fd %d\n", get_Socket());
			printf("slen<0 errno!=EAGAIN\n");
#endif // DEBUG
			close_socket();
			return false;
		}
		if (slen < 0)
		{
			/*	printf("%d %d %d\n ", send_head, send_tail,bufflen);
				assert(0);*/
		
			memcpy(&send_buffer[send_tail], &data[data_head], (size_t)bufflen);
			send_tail += bufflen;
			return true;
		}
	}

	return true;
}
bool ConnSocket::Recv()
{
	if (socket_fd == -1)return false;
	if (recv_head == recv_tail)
	{
		recv_head = recv_tail = 0;
	}
	int rlen = 0;
	do
	{
		if (recv_tail == recv_buffer_size)
		{
			if (recv_head > 0)
			{
				/*	printf("recv head %d recv tail %d\n" ,recv_head,recv_tail);
					assert(0);*/
				memmove(&recv_buffer[0], &recv_buffer[recv_head], recv_tail - recv_head);
				recv_tail = recv_tail - recv_head;
				recv_head = 0;
			}
			else
				return false;
		}
		int bufflen = recv_buffer_size - recv_tail;
		rlen = recv(socket_fd, &recv_buffer[recv_tail], bufflen, 0);
#ifdef DEBUG
		//recv_buffer[rlen + recv_tail] = '\0';
		printf("recv len %d xxx %c xxx\n", rlen, recv_buffer[recv_head]);
#endif // DEBUG
		if (rlen > 0)
		{
			recv_tail += rlen;
		}
		if (rlen == 0)
		{
			close_socket();
			return false;
		}
		if (rlen < 0 && errno != EAGAIN)
		{
			close_socket();
			return false;
		}
	} while (rlen > 0);
	return true;
}
bool ConnSocket::get_one_message(char *data, uint32_t &len)
{
	if(socket_fd == -1)return false;
	if (recv_tail - recv_head >= PROTOCOLHEADLENGTH)
	{
		len = (uint32_t)ntohl((uint32_t)(*(uint32_t *)&recv_buffer[recv_head])) + PROTOCOLHEADLENGTH;
		if (len > recv_tail - recv_head)
		{
			return false;
		}
		else
		{
			//recv_head+=4;
			memcpy(data, &recv_buffer[recv_head], len);
			recv_head += len;
#ifdef DEBUG

			printf("head %d  tail %d len %d\n", recv_head, recv_tail, len);
#endif // DEBUG
			if (recv_head == recv_tail)
			{
#ifdef DEBUG
				printf("head == tail  recv %s\n", data);
#endif // DEBUG
				recv_head = recv_tail = 0;
			}
			return true;
		}
	}
	else
		return false;
}

double ConnSocket::occupancy()
{
	return (send_tail - send_head + 0.0) / send_buffer_size;
}



