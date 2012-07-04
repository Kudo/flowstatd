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
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include "flowstatd.h"
#include "fttime.h"
#include "netflow_handler_v5.h"

int NfHandlerInitV5Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV5_t *nfHandler = (NetflowHandlerV5_t *)this;
    return 1;
}

int NfHandlerUnInitV5Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV5_t *nfHandler = (NetflowHandlerV5_t *)this;
    return 1;
}

static int IsValidPacket(const char *packetBuf, int packetLen)
{
    int recCount;
    struct NF_V5_header *header;

    if ((packetLen - NF_V5_HEADER_SIZE) % NF_V5_RECORD_SIZE != 0)
    {
	Warn("Warning: Invalid Netflow V5 packet.");
	return 0;
    }

    recCount = (packetLen - NF_V5_HEADER_SIZE) / NF_V5_RECORD_SIZE;

    header = (struct NF_V5_header *) packetBuf;

    if (ntohs(header->count) != recCount)
    {
	Warn("Warning: Invalid Netflow V5 packet.");
	return 0;
    }

    return recCount;
}

static void InsertFlowEntry(const char *packetBuf, int recCount)
{
    int i;
    int octets;
    const char *ptr;
    struct NF_V5_header *header;
    struct NF_V5_record *record;

    header = (struct NF_V5_header *) packetBuf;
    ptr = packetBuf + NF_V5_HEADER_SIZE;

    for (i = 0 ; i < recCount; i++)
    {
	NfTimeInfo_t nfTimeInfo;
	ptr += NF_V5_RECORD_SIZE;
	record = (struct NF_V5_record *) ptr;

	nfTimeInfo.SysUptime = ntohl(header->SysUptime);
	nfTimeInfo.UnixSecs = ntohl(header->unix_secs);
	nfTimeInfo.UnixNsecs = ntohl(header->unix_nsecs);
	nfTimeInfo.FirstPacketTime = ntohl(record->First);
	octets = ntohl(record->dOctets);
	InsertNfInfoToIpTable(record->srcaddr, record->dstaddr, octets, &nfTimeInfo);
    }
}

int AddFlowDataV5Impl(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr)
{
    //NetflowHandlerV5_t *nfHandler = (NetflowHandlerV5_t *)this;
    int recCount = IsValidPacket(packetBuf, packetLen);
    if (recCount > 0)
	InsertFlowEntry(packetBuf, recCount);
    return 1;
}

NetflowHandlerFunc_t *NewNetflowHandlerV5()
{
    NetflowHandlerV5_t *nfHandler = (NetflowHandlerV5_t *) malloc(sizeof(NetflowHandlerV5_t));
    nfHandler->funcs.Init = NfHandlerInitV5Impl;
    nfHandler->funcs.UnInit = NfHandlerUnInitV5Impl;
    nfHandler->funcs.AddFlowData = AddFlowDataV5Impl;
    return &(nfHandler->funcs);
}

int FreeNetflowHandlerV5(NetflowHandlerFunc_t *this)
{
    this->UnInit(this);
    NetflowHandlerV5_t *nfHandler = (NetflowHandlerV5_t *)this;
    if (nfHandler != NULL)
    {
	free(nfHandler);
	return 1;
    }
    else
    {
	return 0;
    }
}

