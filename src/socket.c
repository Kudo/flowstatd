#include "flowd.h"

extern int netflowSockFd;
extern int flowdSockFd;
extern int peerFd;
extern MultiplexerFunc_t *multiplexer;

void SendBufToSock(int sckfd, const char *buf, int len)
{
    int bytessent, pos;

    pos = 0;
    do {
	if ((bytessent = send(sckfd, buf + pos, len - pos, 0)) < 0)
	    Diep("send()");
	pos += bytessent;
    } while (bytessent > 0);
}

int BuildUDPSock(in_addr_t listen_ipaddr, uint16_t listen_port)
{
    int sockfd;
    struct sockaddr_in sin;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	Diep("socket() error");

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = listen_ipaddr;
    sin.sin_port = listen_port;

    if (bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	Diep("bind() error");

    return sockfd;
}

int BuildTCPSock(in_addr_t listen_ipaddr, uint16_t listen_port)
{
    int sockfd;
    struct sockaddr_in sin;
    int optVal = 1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	Diep("socket() error");

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = listen_ipaddr;
    sin.sin_port = listen_port;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) == -1)
	Diep("setsockopt() error");

    if (bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	Diep("bind() error");

    if (listen(sockfd, 2) == -1)
	Diep("listen() error");

    return sockfd;
}


void Exits(int s)
{
    close(netflowSockFd);
    close(flowdSockFd);
    FreeMultiplexer(select, multiplexer);
    ExportRecord(TODAY);
    free(ipTable);
    exit(EXIT_SUCCESS);
}

void SockExit(int s)
{
    SendBufToSock(peerFd, "Timeout.\n", 9);
    close(netflowSockFd);
    close(flowdSockFd);
    FreeMultiplexer(select, multiplexer);
    free(ipTable);
    exit(EXIT_SUCCESS);
}
