/*
    flowstatd - Netflow statistics daemon
    Copyright (C) 2012 Kudo Chien <ckchien@gmail.com>

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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netinet/in.h>

void SendBufToSock(int sckfd, const char *buf, int len);
int BuildUDPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
int BuildTCPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
void Exits(int s);
void SockExit(int s);

#endif	    /* _SOCKET_H_ */
