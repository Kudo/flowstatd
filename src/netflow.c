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

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "liblogger/liblogger.h"
#include "fttime.h"
#include "netflow.h"
#include "netflow_handler_v5.h"
#include "netflow_handler_v9.h"
#include "flowstatd.h"

NetflowHandlerFunc_t *g_nfHandlerV5 = NULL;
NetflowHandlerFunc_t *g_nfHandlerV9 = NULL;

int NetflowHandlerInit()
{
    g_nfHandlerV5 = NewNetflowHandlerV5();
    g_nfHandlerV5->Init(g_nfHandlerV5);
    g_nfHandlerV9 = NewNetflowHandlerV9();
    g_nfHandlerV9->Init(g_nfHandlerV9);
    return 1;
}

int NetflowHandlerUnInit()
{
    FreeNetflowHandlerV5(g_nfHandlerV5);
    FreeNetflowHandlerV9(g_nfHandlerV9);
    return 1;
}

int AddFlowData(const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr)
{
    uint16_t *version = (uint16_t *) packetBuf;
    NetflowHandlerFunc_t *nfHandler = NULL;
    switch (ntohs(*version))
    {
	case 9:
	    nfHandler = g_nfHandlerV9;
	    break;
	case 5:
	    nfHandler = g_nfHandlerV5;
	    break;
	default:
	    {
		return 0;
	    }
	    break;
    }

    nfHandler->AddFlowData(nfHandler, packetBuf, packetLen, sourceAddr);

    return 1;
}

inline int getIPIdx(in_addr_t ipaddr)
{
    uchar isHit = 0;
    uint idx = 0;
    int leftPos = 0;
    int rightPos = nSubnet - 1;

    while (leftPos <= rightPos)
    {
	int midPos = leftPos + ((rightPos - leftPos) / 2);
	in_addr_t ipAddrNet = ipaddr & rcvNetList[midPos].mask;

	if (ipAddrNet > rcvNetList[midPos].net)
	    leftPos = midPos + 1;
	else if (ipAddrNet < rcvNetList[midPos].net)
	    rightPos = midPos - 1;
	else
	{
	    int k;
	    for (k = 0; k < (int) midPos; ++k)
		idx += rcvNetList[k].ipCount;

	    idx += (ipaddr & ~rcvNetList[midPos].mask) >> rcvNetList[midPos].maskBits;
	    isHit = 1;
	    break;
	}
    }

    if (isHit > 0)
	return idx;

    if (myNet.net != 0 && myNet.net == (ipaddr & myNet.mask))
	return RET_IN_MYNET;

    return RET_NOT_IN_MYNET;
}

void displayFlowEntry(in_addr_t srcaddr, in_addr_t dstaddr, uint32_t octets)
{
    if (debug)
    {
	char src_ip[17];
	char dst_ip[17];
	struct in_addr src_addr, dst_addr;

	src_addr.s_addr = srcaddr;
	dst_addr.s_addr = dstaddr;
	inet_ntop(PF_INET, (void *) &src_addr, src_ip, 16);
	inet_ntop(PF_INET, (void *) &dst_addr, dst_ip, 16);

	LogInfo("%-17.17s -> %-17.17s Octets: %u", src_ip, dst_ip, octets);
    }
}

struct tm ConvertNfTime(NfTimeInfo_t *nfTimeInfo)
{
    struct fttime ftt = ftltime(nfTimeInfo->SysUptime, nfTimeInfo->UnixSecs, nfTimeInfo->UnixNsecs, nfTimeInfo->FirstPacketTime);
    struct tm tmTime;
    localtime_r((time_t *) &ftt.secs, &tmTime);
    return tmTime;
}

int InsertNfInfoToIpTable(in_addr_t srcAddr, in_addr_t dstAddr, unsigned int flowBytes, NfTimeInfo_t *nfTimeInfo)
{
    int srcIPIdx = getIPIdx(srcAddr);
    int dstIPIdx = getIPIdx(dstAddr);
    struct tm tmTime = ConvertNfTime(nfTimeInfo);

    if (srcIPIdx >= 0 && dstIPIdx == RET_NOT_IN_MYNET)
    {
	if (tmTime.tm_mday == localtm.tm_mday)
	{
	    displayFlowEntry(srcAddr, dstAddr, flowBytes);
	    ipTable[srcIPIdx].sin_addr.s_addr = srcAddr;
	    ipTable[srcIPIdx].hflow[tmTime.tm_hour][UPLOAD] += flowBytes;
	    ipTable[srcIPIdx].nflow[UPLOAD] += flowBytes;
	    ipTable[srcIPIdx].nflow[SUM] += flowBytes;
	}
    }
    else if (dstIPIdx >= 0 && srcIPIdx == RET_NOT_IN_MYNET)
    {
	if (tmTime.tm_mday == localtm.tm_mday)
	{
	    displayFlowEntry(srcAddr, dstAddr, flowBytes);
	    ipTable[dstIPIdx].sin_addr.s_addr = dstAddr;
	    ipTable[dstIPIdx].hflow[tmTime.tm_hour][DOWNLOAD] += flowBytes;
	    ipTable[dstIPIdx].nflow[DOWNLOAD] += flowBytes;
	    ipTable[dstIPIdx].nflow[SUM] += flowBytes;
	}
    }
    return 1;
}
