CPPFLAGS= -g -std=c++11 -pthread 
all: server client
server:TcpSocket.o Main.o MyEpoll.o Server.o common_socket.o
	g++  $(CPPFLAGS) -o server TcpSocket.o Main.o MyEpoll.o Server.o common_socket.o
client:TcpSocket.o client.o common_socket.o
	g++  $(CPPFLAGS)  -o client client.o TcpSocket.o common_socket.o
TcpSocket.o:TcpSocket.cpp
	g++ $(CPPFLAGS) -c TcpSocket.cpp
common_socket.o:common_socket.cpp
	g++ $(CPPFLAGS) -c common_socket.cpp
Main.o:Main.cpp
	g++ $(CPPFLAGS) -c Main.cpp
Server.o:Server.cpp
	g++ $(CPPFLAGS) -c Server.cpp
MyEpoll.o:MyEpoll.cpp
	g++ $(CPPFLAGS) -c MyEpoll.cpp
client.o:client.cpp
	g++ $(CPPFLAGS) -c client.cpp
clean:
	rm -rf *.o server client 
