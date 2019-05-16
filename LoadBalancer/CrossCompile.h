#pragma once

#ifdef WIN32

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib")
int SockAddrInSize = 0x10;

void SetNonBlocking(SOCKET sock) {
	unsigned long iMode = 1;
	ioctlsocket(sock, FIONBIO, &iMode);
}

void SetBlocking(SOCKET sock) {
	unsigned long iMode = 0;
	ioctlsocket(sock, FIONBIO, &iMode);
}

#define ReuseSockets(sock)

#define CheckTimeout(sock, fd_set, tv) FD_ISSET(sock, &Write)
#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SD_BOTH SHUT_RDWR
#define NO_ERROR 0L
#define MAKEWORD(a, b) 2

struct WSAData {
	int nulldata;
};

int WSAStartup(short, void*) {
	return 0;
}

#define closesocket(sock) close(sock)
#define Sleep(time) usleep(time*1000)
#define WSAGetLastError() errno
socklen_t SockAddrInSize = 0x10;

#define TIMEVAL timeval

void ReuseSockets(SOCKET sock) {
	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

int SocketFlag = 0;
void SetNonBlocking(SOCKET sock) {
	SocketFlag = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, SocketFlag | O_NONBLOCK);
}
void SetBlocking(SOCKET sock) {
	fcntl(sock, F_SETFL, SocketFlag);
}

bool CheckTimeout(int sock, fd_set* fdset, TIMEVAL* tv) {
	if (select(sock + 1, NULL, fdset, NULL, tv) == 1) {
		int so_error;
		socklen_t len = sizeof so_error;
		getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
		return so_error == 0;
	}
	return false;
}

#endif