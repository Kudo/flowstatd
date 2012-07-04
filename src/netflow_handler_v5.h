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

#ifndef _NETFLOW_HANDLER_V5_H_
#define _NETFLOW_HANDLER_V5_H_

#include <sys/types.h>
#include "netflow.h"

#define	NF_V5_HEADER_SIZE	sizeof(struct NF_V5_header)
#define NF_V5_RECORD_SIZE	sizeof(struct NF_V5_record)

struct NF_V5_header {
    uint16_t	version;
    uint16_t	count;
    uint32_t	SysUptime;
    uint32_t	unix_secs;
    uint32_t	unix_nsecs;
    uint32_t	flow_sequence;
    uint8_t	engine_type;
    uint8_t	engine_id;
    uint16_t	reserved;
};

struct NF_V5_record {
    uint32_t	srcaddr;
    uint32_t	dstaddr;
    uint32_t	nexthop;
    uint16_t	input;
    uint16_t	output;
    uint32_t	dPkts;
    uint32_t	dOctets;
    uint32_t	First;
    uint32_t	Last;
    uint16_t	srcport;
    uint16_t	dstport;
    uint8_t	pad1;
    uint8_t	tcp_flags;
    uint8_t	prot;
    uint8_t	tos;
    uint16_t	src_as;
    uint16_t	dst_as;
    uint8_t	src_mask;
    uint8_t	dst_mask;
    uint16_t	pad2;
};

typedef struct _NetflowHandlerV5_t {
    NetflowHandlerFunc_t funcs;
} NetflowHandlerV5_t;


int NfHandlerInitV5Impl(NetflowHandlerFunc_t *this);
int NfHandlerUnInitV5Impl(NetflowHandlerFunc_t *this);
int AddFlowDataV5Impl(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr);
NetflowHandlerFunc_t *NewNetflowHandlerV5();
int FreeNetflowHandlerV5(NetflowHandlerFunc_t *this);

#endif	    /* _NETFLOW_HANDLER_V5_H_ */
