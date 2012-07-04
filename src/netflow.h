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

#ifndef _NETFLOW_H_
#define _NETFLOW_H_

#include <time.h>
#include <netinet/in.h>

typedef struct _NetflowHandlerFunc_t NetflowHandlerFunc_t;
struct _NetflowHandlerFunc_t {
    int (*Init)(NetflowHandlerFunc_t *this);
    int (*UnInit)(NetflowHandlerFunc_t *this);

    int (*AddFlowData)(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr);
};

typedef struct _NfTimeInfo_t {
    uint32_t SysUptime;
    uint32_t UnixSecs;
    uint32_t UnixNsecs;
    uint32_t FirstPacketTime;
} NfTimeInfo_t;

#define	RET_IN_MYNET		    -1
#define RET_NOT_IN_MYNET	    -2

int NetflowHandlerInit();
int NetflowHandlerUnInit();
int AddFlowData(const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr);
inline int getIPIdx(in_addr_t ipaddr);
void displayFlowEntry(in_addr_t srcaddr, in_addr_t dstaddr, uint32_t octets);
struct tm ConvertNfTime(NfTimeInfo_t *nfTimeInfo);
int InsertNfInfoToIpTable(in_addr_t srcAddr, in_addr_t dstAddr, unsigned int flowBytes, NfTimeInfo_t *nfTimeInfo);

#endif	/* _NETFLOW_H_ */
