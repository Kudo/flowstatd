#include "flowd.h"

inline int hash(in_addr_t ipaddr, uchar maskBits)
{
    return ntohs(ipaddr >> maskBits);
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
    int hash_value;
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

	for (j = 0; j < nSubnet; ++j)
	{
#if 1
	    if (((record->srcaddr & rcvNetList[j].mask) == rcvNetList[j].net) && ((record->dstaddr & rcvNetList[j].mask) != rcvNetList[j].net))
#else
		if ((record->srcaddr & rcvNetList[j].mask) == rcvNetList[j].net)
#endif
		{
		    hash_value = hash(record->srcaddr);

		    ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
		    tm = localtime((time_t *) &ftt.secs);

		    //	if (tm->tm_hour == localtm->tm_hour)
		    {
			octets = ntohl(record->dOctets);
			//	  if (record->srcaddr == inet_addr("140.123.238.189"))
			//	    printf("Before: %llu Octets: %u After: %llu\n", hashTable[j][hash_value].hflow[tm->tm_hour][UPLOAD], octets, hashTable[j][hash_value].hflow[tm->tm_hour][UPLOAD] + octets);
			hashTable[j][hash_value].sin_addr.s_addr = record->srcaddr;
			hashTable[j][hash_value].hflow[tm->tm_hour][UPLOAD] += octets;
			hashTable[j][hash_value].nflow[UPLOAD] += octets;
			hashTable[j][hash_value].nflow[SUM] += octets;
		    }
		}
#if 1
		else if (((record->dstaddr & rcvNetList[j].mask) == rcvNetList[j].net) && ((record->srcaddr & rcvNetList[j].mask) != rcvNetList[j].net))
#else
		else if ((record->dstaddr & rcvNetList[j].mask) == rcvNetList[j].net)
#endif
		{
		    hash_value = hash(record->dstaddr);

		    ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
		    tm = localtime((time_t *) &ftt.secs);

		    //	if (tm->tm_hour == localtm->tm_hour)
		    {
			octets = ntohl(record->dOctets);
			//	  if (record->dstaddr == inet_addr("140.123.238.189"))
			//	    printf("Before: %llu Octets: %u After: %llu\n", hashTable[j][hash_value].hflow[tm->tm_hour][UPLOAD], octets, hashTable[j][hash_value].hflow[tm->tm_hour][UPLOAD] + octets);

			hashTable[j][hash_value].sin_addr.s_addr = record->dstaddr;
			hashTable[j][hash_value].hflow[tm->tm_hour][DOWNLOAD] += octets;
			hashTable[j][hash_value].nflow[DOWNLOAD] += octets;
			hashTable[j][hash_value].nflow[SUM] += octets;
		    }
		}
	}
    }
}
