#include "flowd.h"

inline int getIPIdx(in_addr_t ipaddr, uchar maskBits)
{
    uint i;
    uchar isHit = 0;
    uint idx = 0;

    for (i = 0; i < nSubnet; ++i)
    {
	if ((ipaddr & rcvNetList[i].mask) == rcvNetList[i].net)
	{
	    idx += ntohs(ipaddr >> rcvNetList[i].maskBits);
	    isHit = 1;
	    break;
	}
	
	idx += rcvNetList[i].ipCount;
    }

    if (isHit > 0)
	return idx;

    return -1;
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

void PushRecord(char *buf, int recCount)
{
    int i, j;
    int srcIPIdx, dstIPIdx;
    int octets;
    char *ptr;
    struct NF_header *header;
    struct NF_record *record;
    struct fttime ftt;
    struct tm *tm;

    header = (struct NF_header *) buf;
    ptr = buf + NF_HEADER_SIZE;

    for (i = 0 ; i < recCount; i++)
    {
	ptr += NF_RECORD_SIZE;
	record = (struct NF_record *) ptr;

	if (verbose)
	{
	    char src_ip[17];
	    char dst_ip[17];
	    struct in_addr src_addr, dst_addr;

	    src_addr.s_addr = record->srcaddr;
	    dst_addr.s_addr = record->dstaddr;
	    inet_ntop(PF_INET, (void *) &src_addr, src_ip, 16);
	    inet_ntop(PF_INET, (void *) &dst_addr, dst_ip, 16);

	    //if (strcmp(src_ip, "140.123.238.189") == 0 || strcmp(dst_ip, "140.123.238.189") == 0)
	    //if (strcmp(src_ip, "140.123.238.189") == 0)
	    //if (strcmp(dst_ip, "140.123.238.189") == 0)
	    printf("SrcIP: %-17.17s DstIP: %-17.17s Octets: %u\n", src_ip, dst_ip, ntohl(record->dOctets));
	}

	ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
	tm = localtime((time_t *) &ftt.secs);
	octets = ntohl(record->dOctets);

	srcIPIdx = getIPIdx(record->srcaddr, record->src_mask);
	dstIPIdx = getIPIdx(record->dstaddr, record->dst_mask);

#if 1
	if (srcIPIdx >= 0 && dstIPIdx == -1)
#else
	if (srcIPIdx >= 0)
#endif
	{
	    //	if (tm->tm_hour == localtm->tm_hour)
	    {
		//	  if (record->srcaddr == inet_addr("140.123.238.189"))
		//	    printf("Before: %llu Octets: %u After: %llu\n", ipTable[srcIPIdx].hflow[tm->tm_hour][UPLOAD], octets, ipTable[srcIPIdx].hflow[tm->tm_hour][UPLOAD] + octets);
		ipTable[srcIPIdx].sin_addr.s_addr = record->srcaddr;
		ipTable[srcIPIdx].hflow[tm->tm_hour][UPLOAD] += octets;
		ipTable[srcIPIdx].nflow[UPLOAD] += octets;
		ipTable[srcIPIdx].nflow[SUM] += octets;
	    }
	}
#if 1
	else if (dstIPIdx >= 0 && srcIPIdx == -1)
#else
	else if (dstIPIdx >= 0)
#endif
	{

	    //	if (tm->tm_hour == localtm->tm_hour)
	    {
		//	  if (record->dstaddr == inet_addr("140.123.238.189"))
		//	    printf("Before: %llu Octets: %u After: %llu\n", ipTable[dstIPIdx].hflow[tm->tm_hour][UPLOAD], octets, ipTable[dstIPIdx].hflow[tm->tm_hour][UPLOAD] + octets);

		ipTable[dstIPIdx].sin_addr.s_addr = record->dstaddr;
		ipTable[dstIPIdx].hflow[tm->tm_hour][DOWNLOAD] += octets;
		ipTable[dstIPIdx].nflow[DOWNLOAD] += octets;
		ipTable[dstIPIdx].nflow[SUM] += octets;
	    }
	}
    }
}
