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

#define	RET_IN_MYNET	    -1
#define RET_NOT_IN_MYNET	    -2

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

int isValidNFP(const char *buf, int len)
{
    int recCount;
    struct NF_header *header;

    if ((len - NF_HEADER_SIZE) % NF_RECORD_SIZE != 0)
    {
	Warn("Warning: Invalid Netflow V5 packet.");
	return 0;
    }

    recCount = (len - NF_HEADER_SIZE) / NF_RECORD_SIZE;

    header = (struct NF_header *) buf;

    if (ntohs(header->count) != recCount)
    {
	Warn("Warning: Invalid Netflow V5 packet.");
	return 0;
    }

    return recCount;
}

static inline void displayFlowEntry(in_addr_t srcaddr, in_addr_t dstaddr, uint32_t octets)
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

	printf("%-17.17s -> %-17.17s Octets: %u\n", src_ip, dst_ip, ntohl(octets));
    }
}

void InsertFlowEntry(char *buf, int recCount)
{
    int i;
    int srcIPIdx, dstIPIdx;
    int octets;
    char *ptr;
    struct NF_header *header;
    struct NF_record *record;
    struct fttime ftt;
    struct tm tmTime;

    header = (struct NF_header *) buf;
    ptr = buf + NF_HEADER_SIZE;

    for (i = 0 ; i < recCount; i++)
    {
	ptr += NF_RECORD_SIZE;
	record = (struct NF_record *) ptr;

	ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
	localtime_r((time_t *) &ftt.secs, &tmTime);
	octets = ntohl(record->dOctets);

	srcIPIdx = getIPIdx(record->srcaddr);
	dstIPIdx = getIPIdx(record->dstaddr);

	if (srcIPIdx >= 0 && dstIPIdx == RET_NOT_IN_MYNET)
	{
	    if (tmTime.tm_mday == localtm.tm_mday)
	    {
		displayFlowEntry(record->srcaddr, record->dstaddr, record->dOctets);
		ipTable[srcIPIdx].sin_addr.s_addr = record->srcaddr;
		ipTable[srcIPIdx].hflow[tmTime.tm_hour][UPLOAD] += octets;
		ipTable[srcIPIdx].nflow[UPLOAD] += octets;
		ipTable[srcIPIdx].nflow[SUM] += octets;
	    }
	}
	else if (dstIPIdx >= 0 && srcIPIdx == RET_NOT_IN_MYNET)
	{
	    if (tmTime.tm_mday == localtm.tm_mday)
	    {
		displayFlowEntry(record->srcaddr, record->dstaddr, record->dOctets);
		ipTable[dstIPIdx].sin_addr.s_addr = record->dstaddr;
		ipTable[dstIPIdx].hflow[tmTime.tm_hour][DOWNLOAD] += octets;
		ipTable[dstIPIdx].nflow[DOWNLOAD] += octets;
		ipTable[dstIPIdx].nflow[SUM] += octets;
	    }
	}
    }
}
