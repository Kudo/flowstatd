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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <zlib.h>
#include "flowstatd.h"
#include "jansson.h"
#include "command.h"
#include "multiplex.h"
#include "netflow.h"
#include "socket.h"

int netflowSockFd;
int flowstatdSockFd;
int peerFd;
MultiplexerFunc_t *multiplexer;

char savePrefix[100] = {0};
char secretKey[256] = {0};
static char configFile[100];

static int LoadWhitelist(char *fname)
{
    int i = 0;
    int j = 0;
    int length = 0;
    int ret = -1;
    in_addr_t addr;
    json_t *jsonRoot = NULL;
    json_t *jsonData = NULL;
    json_error_t jsonError = {0};

    jsonRoot = json_load_file(fname, 0, &jsonError);
    if (jsonRoot == NULL) {
	fprintf(stderr, "Unable to parse whitelist file. fname[%s] jsonErr[%s@%d]\n", fname, jsonError.text, jsonError.line);
	exit(EXIT_FAILURE);
    }
    if (!json_is_object(jsonRoot)) {
	fprintf(stderr, "Invalid file format\n");
	goto Exit;
    }
    jsonData = json_object_get(jsonRoot, "whitelistIps");
    if (jsonData == NULL || !json_is_array(jsonData)) {
	fprintf(stderr, "Invalid file format\n");
	goto Exit;
    }

    memset(whitelist, 0, sizeof(in_addr_t) * MAX_WHITELIST);

    length = json_array_size(jsonData);
    for (i = 0; i < MAX_WHITELIST && i < length; ++i) {
	json_t *hostValue = json_array_get(jsonData, i);
	if (!json_is_string(hostValue)) {
	    //fprintf(stderr, "Invalid file format\n");
	    continue;
	}

	if ((addr = inet_addr(json_string_value(hostValue))) == INADDR_NONE)
	    continue;
	
	for (j = 0; j < (int) nSubnet; ++j)
	{
	    if ((addr & rcvNetList[j].mask) == rcvNetList[j].net)
		whitelist[i++] = addr;
	}
    }
    ret = i;

Exit:
    if (jsonRoot != NULL) { json_decref(jsonRoot); }
    return ret;
}


void ExportRecord(int mode)
{
    gzFile fpZip;
    uint tmpVal;
    char buf[100];

    if (mode == TODAY)
    {
	snprintf(buf, 99, "%s/flowstatdata.%04d-%02d-%02d.gz", savePrefix, 
		localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday);
    }
    else
    {
	struct tm newtm;
	time_t yesterday;

	yesterday = time(NULL) - 86400;
	localtime_r(&yesterday, &newtm);

	snprintf(buf, 99, "%s/flowstatdata.%04d-%02d-%02d.gz", savePrefix, 
		newtm.tm_year + 1900, newtm.tm_mon + 1, newtm.tm_mday);
    }

    if ((fpZip = gzopen(buf, "wb")) == NULL)
	Diep("gzopen in ExportRecord() error");


    tmpVal = sizeof(struct subnet);
    gzwrite(fpZip, &tmpVal, sizeof(tmpVal));
    gzwrite(fpZip, &nSubnet, sizeof(nSubnet));
    tmpVal = sizeof(struct hostflow);
    gzwrite(fpZip, &tmpVal, sizeof(tmpVal));
    gzwrite(fpZip, &sumIpCount, sizeof(sumIpCount));
    gzwrite(fpZip, rcvNetList, sizeof(struct subnet) * nSubnet);
    gzwrite(fpZip, ipTable, sizeof(struct hostflow) * sumIpCount);

    gzclose(fpZip);
}


int ImportRecord(char *fname)
{
    uint tmpVal;
    gzFile fpZip;

    if (access(fname, R_OK) == -1)
	return 0;

    if ((fpZip = gzopen(fname, "rb")) == NULL)
	Diep("gzopen in ImportRecord() error");

    gzread(fpZip, &tmpVal, sizeof(tmpVal));
    gzread(fpZip, &tmpVal, sizeof(nSubnet));

    if (tmpVal > MAX_SUBNET)
	Diep("subnet count in flow data exceeds the maximum value");
    else
	nSubnet = tmpVal;

    gzread(fpZip, &tmpVal, sizeof(tmpVal));
    gzread(fpZip, &tmpVal, sizeof(sumIpCount));

    if (tmpVal > sumIpCount)
    {
	void *ptr;
	if ((ptr = realloc(ipTable, sizeof(struct hostflow) * tmpVal)) == NULL)
	{
	    free(ipTable);
	    fprintf(stderr, "Failed to reallocate memory %zu bytes\n", sizeof(struct hostflow) * tmpVal);
	    exit(EXIT_FAILURE);
	}
	ipTable = ptr;
	sumIpCount = tmpVal;
    }

    gzread(fpZip, rcvNetList, sizeof(struct subnet) * nSubnet);
    gzread(fpZip, ipTable, sizeof(struct hostflow) * sumIpCount);

    gzclose(fpZip);
    return 1;
}

static void Update(int s)
{
    LoadWhitelist(configFile);

    if (fork() == 0)
    {
	time_t now = time(NULL);
	localtime_r(&now, &localtm);

	ExportRecord(TODAY);
	exit(EXIT_SUCCESS);
    }
}

static void Usage(char *progName)
{
    printf("flowstatd version %d.%d.%d\nUsage: %s [-v] [-f /path/to/config.json]\n", FLOWSTATD_VERSION_MAJOR, FLOWSTATD_VERSION_MINOR, FLOWSTATD_VERSION_BUILD, progName);
    exit(EXIT_SUCCESS);
}

static int LoadConfig(char *fname, in_addr_t *bindIpAddr, uint16_t *netflowBindPort, uint16_t *commandBindPort)
{
    int nSubnet = 0;
    int length = 0;
    int i = 0;
    json_t *jsonRoot = NULL;
    json_t *jsonData = NULL;
    json_error_t jsonError = {0};

    jsonRoot = json_load_file(fname, 0, &jsonError);
    if (jsonRoot == NULL) {
	fprintf(stderr, "Unable to parse config file. fname[%s] jsonErr[%s@%d]\n", fname, jsonError.text, jsonError.line);
	exit(EXIT_FAILURE);
    }
    if (!json_is_object(jsonRoot)) {
	fprintf(stderr, "Invalid file format.\n");
	goto Exit;
    }

    // [0] dataDir
    jsonData = json_object_get(jsonRoot, "dataDir");
    if (jsonData == NULL || !json_is_string(jsonData)) {
	fprintf(stderr, "dataDir is missing.\n");
	goto Exit;
    }
    strncpy(savePrefix, json_string_value(jsonData), sizeof(savePrefix) - 1);
    {
	struct stat st;
	if (stat(savePrefix, &st) == -1) {
	    fprintf(stderr, "stat() failed. savePrefix[%s] error[%s].\n", savePrefix, strerror(errno));
	    goto Exit;
	}

	if (!(S_ISDIR(st.st_mode))) {
	    fprintf(stderr, "dataDir path is not directory. savePrefix[%s].\n", savePrefix);
	    goto Exit;
	}
	if (access(savePrefix, R_OK | W_OK | X_OK) == -1) {
	    fprintf(stderr, "dataDir path is not accesable. savePrefix[%s]. error[%s]\n", savePrefix, strerror(errno));
	    goto Exit;
	}
    }

    // [1] myNetwork
    memset(&myNet, 0, sizeof(struct subnet));
    jsonData = json_object_get(jsonRoot, "myNetwork");
    if (jsonData == NULL || !json_is_string(jsonData)) {
	fprintf(stderr, "myNetwork is missing.\n");
	goto Exit;
    }
    {
	char buf[BUFSIZE] = {0};
	char *ipPtr = NULL, *maskPtr = NULL;
	in_addr_t addr;
	int maskBits = 0;
	strncpy(buf, json_string_value(jsonData), sizeof(buf) - 1);

	if ((ipPtr = strtok(buf, "/\t\n ")) == NULL || (maskPtr = strtok(NULL, "/\t\n ")) == NULL ||
	    (addr = inet_addr(ipPtr)) == INADDR_NONE) {
	    fprintf(stderr, "Invalid myNetwork format.\n");
	    goto Exit;
	}

	maskBits = (uint) strtoul(maskPtr, (char **) NULL, 10);
	if (maskBits < 1 && maskBits > 32) {
	    fprintf(stderr, "Invalid myNetwork format.\n");
	    goto Exit;
	}

	myNet.net = addr;
	myNet.mask = 0xffffffff << (32 - maskBits);
	myNet.maskBits = (uchar) maskBits;
	myNet.ipCount = (myNet.mask ^ 0xffffffff) + 1;
	myNet.mask = htonl(myNet.mask);
    }

    // [2] statedNetworks
    jsonData = json_object_get(jsonRoot, "statedNetworks");
    if (jsonData == NULL || !json_is_array(jsonData)) {
	fprintf(stderr, "statedNetworks is missing.\n");
	goto Exit;
    }

    length = json_array_size(jsonData);
    if (length > MAX_SUBNET) {
	fprintf(stderr, "statedNetworks exceeds maximum numbers, please adjust MAX_SUBNET value..\n");
	goto Exit;
    }
    for (i = 0; i < length; ++i) {
	char buf[BUFSIZE] = {0};
	char *ipPtr = NULL, *maskPtr = NULL;
	in_addr_t addr;
	int maskBits = 0;
	json_t *netValue = NULL;

	netValue = json_array_get(jsonData, i);
	if (!json_is_string(netValue)) {
	    fprintf(stderr, "Invalid statedNetworks format\n");
	    continue;
	}
	strncpy(buf, json_string_value(netValue), sizeof(buf) - 1);

	if ((ipPtr = strtok(buf, "/\t\n ")) == NULL || (maskPtr = strtok(NULL, "/\t\n ")) == NULL ||
	    (addr = inet_addr(ipPtr)) == INADDR_NONE) {
	    fprintf(stderr, "Invalid statedNetworks format.\n");
	    continue;
	}

	maskBits = (uint) strtoul(maskPtr, (char **) NULL, 10);
	if (maskBits < 1 && maskBits > 32) {
	    fprintf(stderr, "Invalid statedNetworks format.\n");
	    continue;
	}

	rcvNetList[nSubnet].net = addr;
	rcvNetList[nSubnet].mask = 0xffffffff << (32 - maskBits);
	rcvNetList[nSubnet].maskBits = (uchar) maskBits;
	rcvNetList[nSubnet].ipCount = (rcvNetList[nSubnet].mask ^ 0xffffffff) + 1;
	sumIpCount += rcvNetList[nSubnet].ipCount;
	rcvNetList[nSubnet].mask = htonl(rcvNetList[nSubnet].mask);

	++nSubnet;
    }

    // [3] secretKey
    jsonData = json_object_get(jsonRoot, "secretKey");
    if (jsonData == NULL || !json_is_string(jsonData)) {
	fprintf(stderr, "secretKey is missing.\n");
	goto Exit;
    }
    strncpy(secretKey, json_string_value(jsonData), sizeof(secretKey) - 1);

    // [4] listenIpAddr
    jsonData = json_object_get(jsonRoot, "listenIpAddr");
    if (jsonData != NULL && json_is_string(jsonData)) {
	*bindIpAddr = inet_addr(json_string_value(jsonData));
    }

    // [5] netflowListenPort
    jsonData = json_object_get(jsonRoot, "netflowListenPort");
    if (jsonData == NULL || !json_is_integer(jsonData)) {
	fprintf(stderr, "netflowListenPort is missing.\n");
	goto Exit;
    }
    *netflowBindPort = htons(json_integer_value(jsonData));

    // [6] commandListenPort
    jsonData = json_object_get(jsonRoot, "commandListenPort");
    if (jsonData == NULL || !json_is_integer(jsonData)) {
	fprintf(stderr, "commandListenPort is missing.\n");
	goto Exit;
    }
    *commandBindPort = htons(json_integer_value(jsonData));


Exit:
    if (jsonRoot != NULL) { json_decref(jsonRoot); }
    return nSubnet;
}

static int RcvNetListCmp(const void *a, const void *b)
{
    struct subnet *tmpA = (struct subnet *) a;
    struct subnet *tmpB = (struct subnet *) b;

    return (int) tmpA->net - tmpB->net;
}

void Warn(const char *msg)
{
    if (verbose)
	fprintf(stderr, "%s\n", msg);
}

void Diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int ch;
    int i, n;
    char buf[BUFSIZE];
    struct sockaddr_in pin;
    socklen_t plen;
    time_t now;
    char preHour;

    in_addr_t bindIpAddr = INADDR_ANY;
    uint16_t netflowBindPort = 0;
    uint16_t commandBindPort = 0;

    int nev;

    verbose = 0;
    daemonMode = 0;
    debug = 0;
    sumIpCount = 0;
    strncpy(configFile, DEF_CONFIG_FILE, 99);

    while ((ch = getopt(argc, argv, "vdDf:")) != -1)
    {
	switch ((char) ch)
	{
	    case 'v':		/* Verbose mode */
		verbose = 1;
		break;

	    case 'd':		/* Debug mode */
		debug = 1;
		break;

	    case 'D':		/* Daemon mode */
		daemonMode = 1;
		break;

	    case 'f':
		strncpy(configFile, optarg, 99);
		break;

	    case '?':
	    default:
		Usage(argv[0]);
		return -1;
	}
    }

    nSubnet = LoadConfig(configFile, &bindIpAddr, &netflowBindPort, &commandBindPort);
    if (nSubnet <= 0)
	return -1;
    LoadWhitelist(configFile);

    ipTable = (struct hostflow *) malloc(sizeof(struct hostflow) * sumIpCount);
    if (ipTable == NULL)
    {
	fprintf(stderr, "Failed to allocate memory %zu bytes\n", sizeof(struct hostflow) * sumIpCount);
	return -1;
    }
    memset(ipTable, 0, sizeof(struct hostflow) * sumIpCount);

    // Sort rcvNetList for binary search
    qsort(rcvNetList, nSubnet, sizeof(struct subnet), RcvNetListCmp);

    now = time(NULL);
    localtime_r(&now, &localtm);
    preHour = localtm.tm_hour;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, &Exits);
    signal(SIGTERM, &Exits);
    signal(SIGHUP, &Update);

    snprintf(buf, BUFSIZE - 1, "%s/flowstatdata.%04d-%02d-%02d.gz", savePrefix, 
	    localtm.tm_year + 1900, localtm.tm_mon + 1, localtm.tm_mday);

    ImportRecord(buf);
    setvbuf(stdout, NULL, _IONBF, 0);

    netflowSockFd = BuildUDPSock(bindIpAddr, netflowBindPort);
    if (bigsockbuf(netflowSockFd, SO_RCVBUF, SOCKET_RCV_BUFSIZE) < 0)
	fprintf(stderr, "bigsockbuf() failed\n");
    flowstatdSockFd = BuildTCPSock(bindIpAddr, commandBindPort);

    multiplexer = NewMultiplexer();
    if (multiplexer->Init(multiplexer) == 0)
	Diep("Multiplexer Init() failed");
    
    multiplexer->AddToList(multiplexer, netflowSockFd);
    multiplexer->AddToList(multiplexer, flowstatdSockFd);

    plen = sizeof(struct sockaddr_in);

    if (!verbose && !debug && daemonMode)
    {
	if (daemon(0, 0) == -1)
	{
	    perror("daemon() error");
	    exit(EXIT_FAILURE);
	}
    }


    NetflowHandlerInit();

    while (1)
    {
	nev = multiplexer->Wait(multiplexer);

	for (i = 0; i < nev; i++) 
	{
#if 0
	    if (evlist[i].flags & EV_ERROR) 
	    {
		fprintf(stderr, "kevent() EV_ERROR: %s\n", strerror(evlist[i].data));
		exit(EXIT_FAILURE);
	    }
#endif

	    if (multiplexer->IsActive(multiplexer, netflowSockFd))
	    {
		memset(buf, 0, BUFSIZE);
		if ((n = recvfrom(netflowSockFd, buf, BUFSIZE, 0, (struct sockaddr *) &pin, &plen)) == -1)
		{
		    perror("recvfrom() error");
		    continue;
		}

		now = time(NULL);
		localtime_r(&now, &localtm);

		if (preHour == 23 && localtm.tm_hour == 0)
		{
		    ExportRecord(YESTERDAY);
		    memset(ipTable, 0, sizeof(struct hostflow) * sumIpCount);
		    preHour = localtm.tm_hour;
		}
		else
		{
		    preHour = localtm.tm_hour;
		}

                AddFlowData(buf, n, &pin);
	    }
	    else if (multiplexer->IsActive(multiplexer, flowstatdSockFd))
	    {
		if ((peerFd = accept(flowstatdSockFd, (struct sockaddr *) &pin, &plen)) == -1)
		{
		    Diep("accept() error");
		    continue;
		}

		if (fork() == 0)
		{
		    int length = 0;
		    char command[BUFSIZE] = {0};
		    signal(SIGALRM, &SockExit);
		    alarm(30);
		    
		    while ((n = recv(peerFd, buf, sizeof(buf) - 1, 0)) > 0) {
			if (length + n > sizeof(command) - 1) {
			    fprintf(stderr, "Socket receiver exceeds maximum size\n");
			    n = sizeof(command) - 1 - length;
			    strncat(command + length, buf, n);
			    break;
			}
			strncat(command + length, buf, n);
			length += n;
		    }
		    parseCmd(command);
		    exit(EXIT_SUCCESS);
		}
		close(peerFd);
	    }
	}
    }

    NetflowHandlerUnInit();
    FreeMultiplexer(multiplexer);
    close(netflowSockFd);
    close(flowstatdSockFd);
    free(ipTable);

    return 0;
}

