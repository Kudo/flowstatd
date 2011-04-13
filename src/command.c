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

extern int peerFd;
extern char savePrefix[100];

static void GetOldByIP(int year, int month, int day, char *ipaddr, char realmode)
{
    int i, j;
    int isInSubnet;
    unsigned long currSumIp = 0;
    char buf[BUFSIZE];
    gzFile fpZip;
    struct hostflow ipFlow;
    unsigned long long int totalFlow = 0;
    in_addr_t ipaddr_in = inet_addr(ipaddr);

    snprintf(buf, BUFSIZE - 1,"%s/flowdata.%04d-%02d-%02d.gz", savePrefix, year, month, day);

    if ((fpZip = gzopen(buf, "rb")) == NULL)
    {
	snprintf(buf, BUFSIZE - 1, "No data\n");
	SendBufToSock(peerFd, buf, strlen(buf));
	return;
    }

    gzread(fpZip, &isInSubnet, sizeof(isInSubnet));
    gzread(fpZip, &nSubnet, sizeof(nSubnet));
    gzread(fpZip, &isInSubnet, sizeof(isInSubnet));
    gzread(fpZip, &sumIpCount, sizeof(sumIpCount));
    gzread(fpZip, rcvNetList, sizeof(struct subnet) * nSubnet);

    isInSubnet = -1;
    for (i = 0; i < nSubnet; ++i)
    {
	if ((ipaddr_in & rcvNetList[i].mask) == rcvNetList[i].net)
	{
	    isInSubnet = i;
	    currSumIp += (ipaddr_in & ~rcvNetList[i].mask) >> rcvNetList[i].maskBits;
	    break;
	}
	else 
	    currSumIp += rcvNetList[i].ipCount;
    }

    if (isInSubnet < 0)
    {
	snprintf(buf, BUFSIZE - 1, "No data\n");
	SendBufToSock(peerFd, buf, strlen(buf));
	return;
    }

    if (realmode == 0)
    {
	for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++)
	{
	    if (ipaddr_in == whitelist[j])
	    {
		snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %4d-%02d-%02d\nSUM FLOW: %-12.6f (MB)\n", ipaddr, year, month, day, (double) 0);
		strcat(buf, "-----------------------------------------------------------------------\n");
		strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)        Total (MB)\n");
		strcat(buf, "-----------------------------------------------------------------------\n");
		SendBufToSock(peerFd, buf, strlen(buf));

		for (i = 0; i < 24; i++)
		{
		    snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\t%-12.6f\n", i, (double) 0, (double) 0, (double) 0, (double) 0);
		    SendBufToSock(peerFd, buf, strlen(buf));
		}
		return;
	    }
	}
    }

    gzseek(fpZip, sizeof(struct hostflow) * currSumIp, SEEK_CUR);
    gzread(fpZip, &ipFlow, sizeof(struct hostflow));

    snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %4d-%02d-%02d\nSUM FLOW: %-12.6f (MB)\n", ipaddr, year, month, day,
	    ((double) ipFlow.nflow[SUM]) / MBYTES);
    strcat(buf, "-----------------------------------------------------------------------\n");
    strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)        Total (MB)\n");
    strcat(buf, "-----------------------------------------------------------------------\n");

    SendBufToSock(peerFd, buf, strlen(buf));

    for (i = 0; i < 24; i++)
    {
	totalFlow += ipFlow.hflow[i][UPLOAD] + ipFlow.hflow[i][DOWNLOAD];
	snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\t%-12.6f\n", i,
		((double) (ipFlow.hflow[i][UPLOAD])) / MBYTES,
		((double) (ipFlow.hflow[i][DOWNLOAD])) / MBYTES,
		((double) (ipFlow.hflow[i][UPLOAD] + ipFlow.hflow[i][DOWNLOAD])) / MBYTES,
		((double) (totalFlow)) / MBYTES);
	SendBufToSock(peerFd, buf, strlen(buf));
    }

    gzclose(fpZip);
    return;
}

static void GetByIP(char *ipaddr, char realmode)
{
    int i, j;
    int ipIdx;
    in_addr_t ipaddr_in;
    char buf[BUFSIZE];
    char time[20];
    char isOurNet = 0;
    unsigned long long int totalFlow = 0;

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
		memset(&time, 0, sizeof(time));
		strftime(time, sizeof(time) - 1, "%F %T", &localtm);
		snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, time, (double) 0);
		strcat(buf, "-----------------------------------------------------------------------\n");
		strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)        Total (MB)\n");
		strcat(buf, "-----------------------------------------------------------------------\n");
		SendBufToSock(peerFd, buf, strlen(buf));

		for (i = 0; i < 24; i++)
		{
		    snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\t%-12.6f\n", i, (double) 0, (double) 0, (double) 0, (double) 0);
		    SendBufToSock(peerFd, buf, strlen(buf));
		}

		return;
	    }
	}
    }

    ipIdx = getIPIdx(ipaddr_in);

    memset(&time, 0, sizeof(time));
    strftime(time, sizeof(time) - 1, "%F %T", &localtm);
    snprintf(buf, BUFSIZE - 1, "IP: %s\nTime: %s\nSUM FLOW: %-12.6f (MB)\n", ipaddr, time,
	    ((double) ipTable[ipIdx].nflow[SUM]) / MBYTES);
    strcat(buf, "-----------------------------------------------------------------------\n");
    strcat(buf, "HOUR    UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)        Total (MB)\n");
    strcat(buf, "-----------------------------------------------------------------------\n");
    SendBufToSock(peerFd, buf, strlen(buf));
    for (i = 0; i < 24; i++)
    {
	totalFlow += ipTable[ipIdx].hflow[i][UPLOAD] + ipTable[ipIdx].hflow[i][DOWNLOAD];

	snprintf(buf, BUFSIZE - 1, "%-2.2d\t%-12.6f\t%-12.6f\t%-12.6f\t%-12.6f\n", i,
		((double) (ipTable[ipIdx].hflow[i][UPLOAD])) / MBYTES,
		((double) (ipTable[ipIdx].hflow[i][DOWNLOAD])) / MBYTES,
		((double) (ipTable[ipIdx].hflow[i][UPLOAD] + ipTable[ipIdx].hflow[i][DOWNLOAD])) / MBYTES,
		((double) (totalFlow)) / MBYTES);
	SendBufToSock(peerFd, buf, strlen(buf));
    }
}

static int HostFlowCmp(const void *a, const void *b)
{
    register struct hostflow *recA = (struct hostflow *) a;
    register struct hostflow *recB = (struct hostflow *) b;

    if (recB->nflow[SUM] > recA->nflow[SUM])
	return 1;
    else if (recB->nflow[SUM] == recA->nflow[SUM])
	return 0;
    else
	return -1;
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

    memset(&time, 0, sizeof(time));
    strftime(time, sizeof(time) - 1, "%F %T", &localtm);
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

    snprintf(buf, BUFSIZE - 1, "%s/flowdata.%04d-%02d-%02d.gz", savePrefix, year, month, day);

    if (ImportRecord(buf) == 0)
    {
	SendBufToSock(peerFd, "No data\n", 8);
	return;
    }

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    snprintf(buf, BUFSIZE - 1, "Time: %4d-%02d-%02d\nNo.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n", year, month, day);
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

    memset(&time, 0, sizeof(time));
    strftime(time, sizeof(time) - 1, "%F %T", &localtm);
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

    snprintf(buf, BUFSIZE - 1, "%s/flowdata.%04d-%02d-%02d.gz", savePrefix, year, month, day);

    if (ImportRecord(buf) == 0)
    {
	SendBufToSock(peerFd, "No data\n", 8);
	return;
    }

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    snprintf(buf, BUFSIZE - 1, "Time: %4d-%02d-%02d\nNo.     IP                      UPLOAD (MB)     DOWNLOAD (MB)   SUM (MB)\n", year, month, day);
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

