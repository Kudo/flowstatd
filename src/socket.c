/*
    flowd - Netflow statistics daemon
    Copyright (C) 2011 Kudo Chien <ckchien@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Optionally you can also view the license at <http://www.gnu.org/licenses/>.
*/

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
    FreeMultiplexer(multiplexer);
    ExportRecord(TODAY);
    free(ipTable);
    exit(EXIT_SUCCESS);
}

void SockExit(int s)
{
    SendBufToSock(peerFd, "Timeout.\n", 9);
    close(netflowSockFd);
    close(flowdSockFd);
    FreeMultiplexer(multiplexer);
    free(ipTable);
    exit(EXIT_SUCCESS);
}
