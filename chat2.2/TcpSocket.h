
#ifndef TcpSocketh
#define TcpSocketh
#include <signal.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/select.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <time.h>
#include <errno.h>
const int recv_buffer_size = 1000000;
const int send_buffer_size = 1000000;


class Socket
{
public:
	Socket(int fd);
	Socket();
	~Socket();

	int get_Socket();
	void set_Socket(int fd);
	void close_socket();
	bool is_open();
protected:
	int socket_fd;
};
class ListenSocket :public Socket
{
public:
	ListenSocket();
	ListenSocket(int fd);
	~ListenSocket()
	{
	};

	bool open_server(u_short port, const char* ip);


private:

};
class ConnSocket :public Socket
{
public:
	ConnSocket();
	ConnSocket(int fd);
	~ConnSocket()
	{
	};
	bool Connect(const char *ip, u_short port, int ms);
	bool Send(char *data, int size);
	bool Send_Buffer();
	bool Recv();
	void set_Socket(int fd);
	bool get_one_message(char *data, uint32_t &len);
	double occupancy();
private:
	char recv_buffer[recv_buffer_size + 1], send_buffer[send_buffer_size + 1];
	int recv_head;
	int recv_tail;
	int send_head;
	int send_tail;
};
#endif