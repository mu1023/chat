
#ifndef COMMONSOCKETH
#define COMMONSOCKETH
#include <fcntl.h>
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
#include<string>
#include <sys/timeb.h>
#include <time.h>
#include <errno.h>
#define PROTOCOLHEADLENGTH 8
enum MESSAGETYPE
{
	MESSAGEBROADCAST = 0,//广播
	MESSAGEECHO,//回显
	MESSAGEEXIT,//关闭
	MESSAGEERROR
};
bool set_nonblock(int fd);
bool ProtocolFormat(uint32_t datatype, const char *msg1, char* msg2, uint32_t &len);
MESSAGETYPE getMessageType(char *data);
std::string RandomName();
#endif 