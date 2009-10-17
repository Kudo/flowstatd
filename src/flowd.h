#ifndef _FLOWD_H_
#define _FLOWD_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include <sys/event.h>
#include <sys/time.h>

//#define	MBYTES		1048576
#define MBYTES		1000000
#define	BUFSIZE		8192

#define	MAX_WHITELIST	50
#define MAX_SUBNET	50

#define	UPLOAD		0
#define	DOWNLOAD	1
#define	SUM		2

#define	DEF_SAVE_PREFIX		"../data"
#define	DEF_SUBNET_FILE		"../etc/subnet.conf"
#define	DEF_WHITELIST		"../etc/whitelist"
#define NETFLOW_LISTEN_PORT	9991
#define FLOWD_LISTEN_PORT	9000
#define	SECRET_KEY		"secret"

#define	TODAY		0
#define	YESTERDAY	1

/*
 * Data Struecture
 */
typedef unsigned int uint;
typedef unsigned char uchar;

struct hostflow {
    struct in_addr sin_addr;
    unsigned long long int hflow[24][2];       // 24 hours,  Upload/Download
    unsigned long long int nflow[3];           // flow now,Upload/Download/Sum
};

struct subnet {
    in_addr_t net;
    in_addr_t mask;
    uchar maskBits;
    uint ipCount;
};

/*
 * From flow-tools library
 */
struct fttime {
    uint secs;
    uint msecs;
};

#define	NF_HEADER_SIZE	sizeof(struct NF_header)
#define NF_RECORD_SIZE	sizeof(struct NF_record)

struct NF_header {
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

struct NF_record {
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

/*
 * Global variables
 */
extern int verbose;
extern struct tm localtm;
extern struct hostflow *ipTable;
extern struct hostflow *hashTable;
extern struct subnet rcvNetList[MAX_SUBNET];
extern in_addr_t whitelist[MAX_WHITELIST];
extern uint nSubnet;
extern uint sumIpCount;

#include "flowd.p"

#endif
