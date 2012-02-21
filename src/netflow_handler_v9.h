/*
    flowd - Netflow statistics daemon
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

#ifndef _NETFLOW_HANDLER_V9_H_
#define _NETFLOW_HANDLER_V9_H_

#include <sys/types.h>
#include "netflow.h"

#define	NF_V9_HEADER_SIZE	sizeof(struct NF_V9_header)

struct NF_V9_header {
    uint16_t	version;
    uint16_t	count;
    uint32_t	SysUptime;
    uint32_t	unix_secs;
    uint32_t	flow_sequence;
    uint32_t	source_id;
};

struct NF_V9_flowset_header
{
    uint16_t	flowset_id;
    uint16_t	length;
}; 

struct NF_V9_flowset_record
{
    uint16_t	type;
    uint16_t	length;
};

struct NF_V9_template_header
{
    uint16_t	template_id;
    uint16_t	field_count;
}; 

struct NF_V9_options_template_header
{
    uint16_t	template_id;
    uint16_t	options_scope_length;
    uint16_t	options_length;
}; 

/* Field Type Definitions */
#define NF_V9_FIELD_TYPE_IN_BYTES		1
#define NF_V9_FIELD_TYPE_IN_PKTS		2
#define NF_V9_FIELD_TYPE_FLOWS			3
#define NF_V9_FIELD_TYPE_PROTOCOL		4
#define NF_V9_FIELD_TYPE_SRC_TOS		5
#define NF_V9_FIELD_TYPE_TCP_FLAGS		6
#define NF_V9_FIELD_TYPE_L4_SRC_PORT		7
#define NF_V9_FIELD_TYPE_IPV4_SRC_ADDR		8
#define NF_V9_FIELD_TYPE_SRC_MASK		9
#define NF_V9_FIELD_TYPE_INPUT_SNMP		10
#define NF_V9_FIELD_TYPE_L4_DST_PORT		11
#define NF_V9_FIELD_TYPE_IPV4_DST_ADDR		12
#define NF_V9_FIELD_TYPE_DST_MASK		13
#define NF_V9_FIELD_TYPE_OUTPUT_SNMP		14
#define NF_V9_FIELD_TYPE_IPV4_NEXT_HOP		15
#define NF_V9_FIELD_TYPE_SRC_AS			16
#define NF_V9_FIELD_TYPE_DST_AS			17
#define NF_V9_FIELD_TYPE_BGP_IPV4_NEXT_HOP	18
#define NF_V9_FIELD_TYPE_MUL_DST_PKTS		19
#define NF_V9_FIELD_TYPE_MUL_DST_BYTES		20
#define NF_V9_FIELD_TYPE_LAST_SWITCHED		21
#define NF_V9_FIELD_TYPE_FIRST_SWITCHED		22
#define NF_V9_FIELD_TYPE_OUT_BYTES		23
#define NF_V9_FIELD_TYPE_OUT_PKTS		24
#define NF_V9_FIELD_TYPE_MIN_PKT_LNGTH		25
#define NF_V9_FIELD_TYPE_MAX_PKT_LNGTH		26
#define NF_V9_FIELD_TYPE_IPV6_SRC_ADDR		27
#define NF_V9_FIELD_TYPE_IPV6_DST_ADDR		28
#define NF_V9_FIELD_TYPE_IPV6_SRC_MASK		29
#define NF_V9_FIELD_TYPE_IPV6_DST_MASK		30
#define NF_V9_FIELD_TYPE_IPV6_FLOW_LABEL	31
#define NF_V9_FIELD_TYPE_ICMP_TYPE		32
#define NF_V9_FIELD_TYPE_MUL_IGMP_TYPE		33
#define NF_V9_FIELD_TYPE_SAMPLING_INTERVAL	34
#define NF_V9_FIELD_TYPE_SAMPLING_ALGORITHM	35
#define NF_V9_FIELD_TYPE_FLOW_ACTIVE_TIMEOUT	36
#define NF_V9_FIELD_TYPE_FLOW_INACTIVE_TIMEOUT	37
#define NF_V9_FIELD_TYPE_ENGINE_TYPE		38
#define NF_V9_FIELD_TYPE_ENGINE_ID		39
#define NF_V9_FIELD_TYPE_TOTAL_BYTES_EXP	40
#define NF_V9_FIELD_TYPE_TOTAL_PKTS_EXP		41
#define NF_V9_FIELD_TYPE_TOTAL_FLOWS_EXP	42
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_1	43
#define NF_V9_FIELD_TYPE_IPV4_SRC_PREFIX	44
#define NF_V9_FIELD_TYPE_IPV4_DST_PREFIX	45
#define NF_V9_FIELD_TYPE_MPLS_TOP_LABEL_TYPE	46
#define NF_V9_FIELD_TYPE_MPLS_TOP_LABEL_IP_ADDR	47
#define NF_V9_FIELD_TYPE_FLOW_SAMPLER_ID	48
#define NF_V9_FIELD_TYPE_FLOW_SAMPLER_MODE	49
#define NF_V9_FIELD_TYPE_FLOW_SAMPLER_RANDOM_INTERVAL	50
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_2	51
#define NF_V9_FIELD_TYPE_MIN_TTL		52
#define NF_V9_FIELD_TYPE_MAX_TTL		53
#define NF_V9_FIELD_TYPE_IPV4_IDENT		54
#define NF_V9_FIELD_TYPE_DST_TOS		55
#define NF_V9_FIELD_TYPE_IN_SRC_MAC		56
#define NF_V9_FIELD_TYPE_OUT_DST_MAC		57
#define NF_V9_FIELD_TYPE_SRC_VLAN		58
#define NF_V9_FIELD_TYPE_DST_VLAN		59
#define NF_V9_FIELD_TYPE_IP_PROTOCOL_VERSION	60
#define NF_V9_FIELD_TYPE_DIRECTION		61
#define NF_V9_FIELD_TYPE_IPV6_NEXT_HOP		62
#define NF_V9_FIELD_TYPE_BPG_IPV6_NEXT_HOP	63
#define NF_V9_FIELD_TYPE_IPV6_OPTION_HEADERS	64
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_3	65
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_4	66
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_5	67
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_6	68
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_7	69
#define NF_V9_FIELD_TYPE_MPLS_LABEL_1		70
#define NF_V9_FIELD_TYPE_MPLS_LABEL_2		71
#define NF_V9_FIELD_TYPE_MPLS_LABEL_3		72
#define NF_V9_FIELD_TYPE_MPLS_LABEL_4		73
#define NF_V9_FIELD_TYPE_MPLS_LABEL_5		74
#define NF_V9_FIELD_TYPE_MPLS_LABEL_6		75
#define NF_V9_FIELD_TYPE_MPLS_LABEL_7		76
#define NF_V9_FIELD_TYPE_MPLS_LABEL_8		77
#define NF_V9_FIELD_TYPE_MPLS_LABEL_9		78
#define NF_V9_FIELD_TYPE_MPLS_LABEL_10		79
#define NF_V9_FIELD_TYPE_IN_DST_MAC		80
#define NF_V9_FIELD_TYPE_OUT_SRC_MAC		81
#define NF_V9_FIELD_TYPE_IF_NAME		82
#define NF_V9_FIELD_TYPE_IF_DESC		83
#define NF_V9_FIELD_TYPE_SAMPLER_NAME		84
#define NF_V9_FIELD_TYPE_IN_PERMANENT_BYTES	85
#define NF_V9_FIELD_TYPE_IN_PERMANENT_PKTS	86
#define NF_V9_FIELD_TYPE_VENDOR_PROPRIETARY_8	87
#define NF_V9_FIELD_TYPE_FRAGMENT_OFFSET	88
#define NF_V9_FIELD_TYPE_FORWARDING_STATUS	89
#define NF_V9_FIELD_TYPE_MPLS_PAL_RD		90
#define NF_V9_FIELD_TYPE_MPLS_PREFIX_LEN	91
#define NF_V9_FIELD_TYPE_SRC_TRAFFIC_INDEX	92
#define NF_V9_FIELD_TYPE_DST_TRAFFIC_INDEX	93
#define NF_V9_FIELD_TYPE_APPLICATION_DESCRIPTION	94
#define NF_V9_FIELD_TYPE_APPLICATION_TAG	95
#define NF_V9_FIELD_TYPE_APPLICATION NAME	96
#define NF_V9_FIELD_TYPE_POSTIPDIFFSERVCODEPOINT	98
#define NF_V9_FIELD_TYPE_REPLICATION FACTOR	99
#define NF_V9_FIELD_TYPE_DEPRECATED	100
#define NF_V9_FIELD_TYPE_LAYER2PACKETSECTIONOFFSET	102
#define NF_V9_FIELD_TYPE_LAYER2PACKETSECTIONSIZE	103
#define NF_V9_FIELD_TYPE_LAYER2PACKETSECTIONDATA	104


/* Constant Definitions */
#define NF_V9_MAX_FIELDS_IN_TEMPLATE		128
#define NF_V9_MAX_TEMPLATES			16

#define NF_V9_TEMPLATE_TYPE_NONE		0
#define NF_V9_TEMPLATE_TYPE_TEMPLATE		1
#define NF_V9_TEMPLATE_TYPE_OPTIONS_TEMPLATE	2

/* Data Type Definitions */
struct NF_V9_template_table_entry {
  uint16_t template_type;
  uint16_t field_count;
  struct NF_V9_flowset_record fields[NF_V9_MAX_FIELDS_IN_TEMPLATE];
};

typedef struct _NetflowHandlerV9_t {
    NetflowHandlerFunc_t funcs;
} NetflowHandlerV9_t;

int NfHandlerInitV9Impl(NetflowHandlerFunc_t *this);
int NfHandlerUnInitV9Impl(NetflowHandlerFunc_t *this);
int AddFlowDataV9Impl(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen);
NetflowHandlerFunc_t *NewNetflowHandlerV9();
int FreeNetflowHandlerV9(NetflowHandlerFunc_t *this);

#endif	    /* _NETFLOW_HANDLER_V9_H_ */
