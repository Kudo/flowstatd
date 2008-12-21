#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <machine/param.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define	IP_PREFIX		0x00007b8c		// 140.123.0.0/16
//#define	IP_NUM			7680			// 256 (Class C)   x   3 (一棟最多三個網段)   x    10 (宿舍數)     = 7680
#define	IP_NUM			65536			// Class B

#define	UPLOAD		0
#define	DOWNLOAD	1

#define	SHMKEY		9991

int verbose = 0;
in_addr_t listen_ipaddr;
uint16_t listen_port;
int sockfd;
int shmid;
struct hostflow *hash_table;

struct hostflow {
  struct in_addr sin_addr;
  unsigned long long int hflow[24][2];       // 24 hours,  Upload/Download
  unsigned long long int nflow[3];           // flow now,Upload/Download/Sum
};

/*
 * From flow-tools library
 */
struct fttime {
  int secs;
  int msecs;
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

#include "flowd.p"
