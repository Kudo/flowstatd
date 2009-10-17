#include "flowd.h"

extern int peerFd;
extern char savePrefix[100];

static void GetOldByIP(int year, int month, int day, char *ipaddr, char realmode)
{
    int i, j;
    char *ptr;
    char buf[BUFSIZE];
    FILE *fp;
    in_addr_t ipaddr_in = inet_addr(ipaddr);

    snprintf(buf, BUFSIZE - 1,"%s/flowdata.%04d-%02d-%02d", savePrefix, year, month, day);

    if ((fp = fopen(buf, "r")) == NULL)
    {
	snprintf(buf, BUFSIZE - 1, "Open data file failed: %s\n", strerror(errno));
	SendBufToSock(peerFd, buf, strlen(buf));
	return;
    }

    if (realmode == 0)
    {
	for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	{
	    if (ipaddr_in == whitelist[j])
	    {
		snprintf(buf, BUFSIZE - 1, "IP: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, (double) 0);
		strcat(buf, "--------------------------------------------------------\n");
		strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
		strcat(buf, "--------------------------------------------------------\n");
		SendBufToSock(peerFd, buf, strlen(buf));

		for (i = 0; i < 24; i++)
		{
		    snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\n", i, (double) 0, (double) 0, (double) 0);
		    SendBufToSock(peerFd, buf, strlen(buf));
		}
		return;
	    }
	}
    }

    while (fgets(buf, BUFSIZE - 1, fp))
    {
	/* Num */
	if ((ptr = strtok(buf, " \n\t\r")) == NULL)
	    continue;

	/* IP */
	if (((ptr = strtok(NULL, " \n\t\r")) != NULL) && (strcmp(ptr, ipaddr) == 0))
	{
	    unsigned long long hflow[24][2];
	    unsigned long long nflow[3];

	    for (i = 0; i < 24; i++)
	    {
		ptr = strtok(NULL, " \n\t\r");
		hflow[i][UPLOAD] = strtoull(ptr, (char **) NULL, 10);

		ptr = strtok(NULL, " \n\t\r");
		hflow[i][DOWNLOAD] = strtoull(ptr, (char **) NULL, 10);
	    }

	    ptr = strtok(NULL, " \n\t\r");
	    nflow[UPLOAD] = strtoull(ptr, (char **) NULL, 10);

	    ptr = strtok(NULL, " \n\t\r");
	    nflow[DOWNLOAD] = strtoull(ptr, (char **) NULL, 10);

	    ptr = strtok(NULL, " \n\t\r");
	    nflow[SUM] = strtoull(ptr, (char **) NULL, 10);

	    snprintf(buf, BUFSIZE - 1, "IP: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, 
		    ((double) nflow[SUM]) / MBYTES);
	    strcat(buf, "--------------------------------------------------------\n");
	    strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
	    strcat(buf, "--------------------------------------------------------\n");
	    SendBufToSock(peerFd, buf, strlen(buf));

	    for (i = 0; i < 24; i++)
	    {
		snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\n", i,
			((double) (hflow[i][UPLOAD])) / MBYTES,
			((double) (hflow[i][DOWNLOAD])) / MBYTES,
			((double) (hflow[i][UPLOAD] + hflow[i][DOWNLOAD])) / MBYTES);
		SendBufToSock(peerFd, buf, strlen(buf));
	    }

	    fclose(fp);
	    return;
	}
    }
    SendBufToSock(peerFd, "No data\n", 8);
    fclose(fp);
}

static void GetByIP(char *ipaddr, char realmode)
{
    int i, j;
    int ipIdx;
    in_addr_t ipaddr_in;
    char buf[BUFSIZE];
    char time[20];
    char isOurNet = 0;

    isOurNet = 0;
    for (i = 0; i < (int) nSubnet; ++i)
    {
	if (((ipaddr_in = inet_addr(ipaddr)) & rcvNetList[i].mask) == rcvNetList[i].net)
	{
	    isOurNet = 1;
	    break;
	}
    }

    if (isOurNet == 0)
    {
	SendBufToSock(peerFd, "No data\n", 8);
	return;
    }

    if (realmode == 0)
    {
	for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	{
	    if (ipaddr_in == whitelist[j])
	    {
		strftime(time, 19, "%F %T", &localtm);
		snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, time, (double) 0);
		strcat(buf, "--------------------------------------------------------\n");
		strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
		strcat(buf, "--------------------------------------------------------\n");
		SendBufToSock(peerFd, buf, strlen(buf));

		for (i = 0; i < 24; i++)
		{
		    snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\n", i, (double) 0, (double) 0, (double) 0);
		    SendBufToSock(peerFd, buf, strlen(buf));
		}

		return;
	    }
	}
    }

    ipIdx = getIPIdx(ipaddr_in);

    strftime(time, 19, "%F %T", &localtm);
    snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, time,
	    ((double) ipTable[ipIdx].nflow[SUM]) / MBYTES);
    strcat(buf, "--------------------------------------------------------\n");
    strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
    strcat(buf, "--------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));
    for (i = 0; i < 24; i++)
    {
	snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\n", i,
		((double) (ipTable[ipIdx].hflow[i][UPLOAD])) / MBYTES,
		((double) (ipTable[ipIdx].hflow[i][DOWNLOAD])) / MBYTES,
		((double) (ipTable[ipIdx].hflow[i][UPLOAD] + ipTable[ipIdx].hflow[i][DOWNLOAD])) / MBYTES);
	SendBufToSock(peerFd, buf, strlen(buf));
    }
}

static int HostFlowCmp(const void *a, const void *b)
{
    return (int) (((struct hostflow *) b)->nflow[SUM] - ((struct hostflow *) a)->nflow[SUM]);
}

static void GetByFlow(uint overMB, char realmode)
{
    int i, j;
    uint count = 0;
    char buf[BUFSIZE];
    char ip[17];
    char show;
    char time[20];

    if (overMB <= 0)
	return;

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    strftime(time, 19, "%F %T", &localtm);
    snprintf(buf, BUFSIZE - 1, "Time: %s\nNo.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n", time);
    strcat(buf, "------------------------------------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));

    i = 0;
    while ((ipTable[i].nflow[SUM] / MBYTES) >= overMB)
    {
	show = 1;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, 16);

	if (realmode == 0)
	{
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	    {
		if (ipTable[i].sin_addr.s_addr == whitelist[j])
		    show = 0;
	    }
	}

	if (show == 1)
	{
	    snprintf(buf, BUFSIZE - 1, "%5u\t%-16.16s\t%-12.6f\t%-12.6f\t%-12.6f\n", ++count, ip,
		    ((double) ipTable[i].nflow[UPLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[SUM]) / MBYTES);
	    SendBufToSock(peerFd, buf, strlen(buf));
	}
	++i;
    }
}

static void GetOldByFlow(int year, int month, int day, uint overMB, char realmode)
{
    int i, j;
    uint count = 0;
    char buf[BUFSIZE];
    char ip[17];
    char show;

    if (overMB <= 0)
	return;

    snprintf(buf, BUFSIZE - 1, "%s/flowdata.%04d-%02d-%02d", savePrefix, year, month, day);

    if (ImportRecord(buf) == 0)
    {
	SendBufToSock(peerFd, "No data\n", 8);
	return;
    }

    memset(ipTable, 0, sizeof(struct hostflow) * sumIpCount);
    ImportRecord(buf);

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    snprintf(buf, BUFSIZE - 1, "No.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
    strcat(buf, "------------------------------------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));

    i = 0;
    while ((ipTable[i].nflow[SUM] / MBYTES) >= overMB)
    {
	show = 1;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, 16);

	if (realmode == 0)
	{
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	    {
		if (ipTable[i].sin_addr.s_addr == whitelist[j])
		    show = 0;
	    }
	}

	if (show == 1)
	{
	    snprintf(buf, BUFSIZE - 1, "%5u\t%-16.16s\t%-12.6f\t%-12.6f\t%-12.6f\n", ++count, ip,
		    ((double) ipTable[i].nflow[UPLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[SUM]) / MBYTES);
	    SendBufToSock(peerFd, buf, strlen(buf));
	}
	++i;
    }
}

static void GetByTopN(uint topN, char realmode)
{
    int i, j;
    uint count = 0;
    char buf[BUFSIZE];
    char ip[17];
    char show;
    char time[20];

    if (topN <= 0 || topN >= sumIpCount)
	return;

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    strftime(time, 19, "%F %T", &localtm);
    snprintf(buf, BUFSIZE - 1, "Time: %s\nNo.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n", time);
    strcat(buf, "------------------------------------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));

    i = 0;
    while (count < topN)
    {
	show = 1;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, 16);

	if (realmode == 0)
	{
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	    {
		if (ipTable[i].sin_addr.s_addr == whitelist[j])
		    show = 0;
	    }
	}

	if (show == 1)
	{
	    snprintf(buf, BUFSIZE - 1, "%5u\t%-16.16s\t%-12.6f\t%-12.6f\t%-12.6f\n", ++count, ip,
		    ((double) ipTable[i].nflow[UPLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[SUM]) / MBYTES);
	    SendBufToSock(peerFd, buf, strlen(buf));
	}
	++i;
    }
}

static void GetOldByTopN(int year, int month, int day, uint topN, char realmode)
{
    int i, j;
    uint count = 0;
    char buf[BUFSIZE];
    char ip[17];
    char show;

    if (topN <= 0 || topN >= sumIpCount)
	return;

    snprintf(buf, BUFSIZE - 1, "%s/flowdata.%04d-%02d-%02d", savePrefix, year, month, day);

    if (ImportRecord(buf) == 0)
    {
	SendBufToSock(peerFd, "No data\n", 8);
	return;
    }

    memset(ipTable, 0, sizeof(struct hostflow) * sumIpCount);

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    snprintf(buf, BUFSIZE - 1, "No.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n");
    strcat(buf, "------------------------------------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));

    i = 0;
    while (count < topN)
    {
	show = 1;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, 16);

	if (realmode == 0)
	{
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	    {
		if (ipTable[i].sin_addr.s_addr == whitelist[j])
		    show = 0;
	    }
	}

	if (show == 1)
	{
	    snprintf(buf, BUFSIZE - 1, "%5u\t%-16.16s\t%-12.6f\t%-12.6f\t%-12.6f\n", ++count, ip,
		    ((double) ipTable[i].nflow[UPLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES,
		    ((double) ipTable[i].nflow[SUM]) / MBYTES);
	    SendBufToSock(peerFd, buf, strlen(buf));
	}
	++i;
    }
}

void parseCmd(char *cmd)
{
    char *ptr;

    ptr = strtok(cmd, " \n\t\r");
    if (ptr == NULL)
    {
	SendBufToSock(peerFd, "Invalid Command.\n", 18);
	return;
    }

    // Get Command
    if (strcasecmp(ptr, "get") == 0)
    {
	char *ipPtr;

	ptr = strtok(NULL, " \n\t\r");
	if (ptr == NULL)
	{
	    SendBufToSock(peerFd, "Invalid Command.\n", 18);
	    return;
	}

	ipPtr = ptr;
	if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
	    GetByIP(ipPtr, 1);
	else
	    GetByIP(ipPtr, 0);
    }
    else if (strcasecmp(ptr, "getR") == 0)
    {
	int year, month, day;
	char *ipPtr;

	if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (year = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (month = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (day = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) != NULL && (ipPtr = ptr))
	{
	    if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
		GetOldByIP(year, month, day, ipPtr, 1);
	    else
		GetOldByIP(year, month, day, ipPtr, 0);

	    return;
	}

	SendBufToSock(peerFd, "Invalid Command.\n", 18);
    }
    // Over Command
    else if (strcasecmp(ptr, "over") == 0)
    {
	char *overMBPtr;

	ptr = strtok(NULL, " \n\t\r");
	if (ptr == NULL)
	{
	    SendBufToSock(peerFd, "Invalid Command.\n", 18);
	    return;
	}

	overMBPtr = ptr;
	if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
	    GetByFlow(atoi(overMBPtr), 1);
	else
	    GetByFlow(atoi(overMBPtr), 0);
    }
    else if (strcasecmp(ptr, "overR") == 0)
    {
	int year, month, day, overMB;

	if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (year = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (month = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (day = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) != NULL && (overMB = atoi(ptr)) > 0)
	{
	    if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
		GetOldByFlow(year, month, day, overMB, 1);
	    else
		GetOldByFlow(year, month, day, overMB, 0);

	    return;
	}

	SendBufToSock(peerFd, "Invalid Command.\n", 18);
    }
    // Top Command
    else if (strcasecmp(ptr, "top") == 0)
    {
	char *topNPtr;

	ptr = strtok(NULL, " \n\t\r");
	if (ptr == NULL)
	{
	    SendBufToSock(peerFd, "Invalid Command.\n", 18);
	    return;
	}

	topNPtr = ptr;
	if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
	    GetByTopN(atoi(topNPtr), 1);
	else
	    GetByTopN(atoi(topNPtr), 0);
    }
    else if (strcasecmp(ptr, "topR") == 0)
    {
	int year, month, day, topN;

	if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (year = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (month = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) == NULL || (day = atoi(ptr)) <= 0) ;
	else if ((ptr = strtok(NULL, " \n\t\r")) != NULL && (topN = atoi(ptr)) > 0)
	{
	    if ((ptr = strtok(NULL, " \n\t\r")) != NULL && strcmp(ptr, SECRET_KEY) == 0)
		GetOldByTopN(year, month, day, topN, 1);
	    else
		GetOldByTopN(year, month, day, topN, 0);

	    return;
	}

	SendBufToSock(peerFd, "Invalid Command.\n", 18);
    }
    else
	SendBufToSock(peerFd, "Invalid Command.\n", 18);
}

