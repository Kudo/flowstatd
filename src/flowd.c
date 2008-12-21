#include "flowd.h"

int netflowSockFd;
int flowdSockFd;
int peerFd;
int kq;

char savePrefix[100];
static char subnetFile[100];
static char whitelistFile[100];

static void SetSavePrefix(char *dir)
{
    struct stat st;

    if (strlen(dir) > 99)
	return;

    if (stat(dir, &st) == -1)
    {
	Warn("stat() in SetSavePrefix() error");
	return;
    }

    if (S_ISDIR(st.st_mode) && (access(dir, R_OK | W_OK | X_OK) == 0))
	strncpy(savePrefix, dir, 99);
}

static int LoadWhitelist(char *fname)
{
    int i = 0;
    FILE *fp;
    char buf[20];
    in_addr_t addr;

    memset(whitelist, 0, sizeof(in_addr_t) * MAX_WHITELIST);

    if ((fp = fopen(whitelistFile, "r")) == NULL)
    {
	fprintf(stderr, "Open whitelist file %s failed: %s\n", whitelistFile, strerror(errno));
	exit(EXIT_FAILURE);
    }

    while (i < MAX_WHITELIST && ((fgets(buf, 19, fp)) != NULL))
    {
	// different netmask
	if ((addr = inet_addr(buf)) != INADDR_NONE && ((addr & NETMASK) == IP_PREFIX))
	    whitelist[i++] = addr;
    }

    fclose(fp);

    return i;
}


void ExportRecord(int mode)
{
    int i, j;
    FILE *fp;
    char buf[100];

    if (mode == TODAY)
	snprintf(buf, 99, "%s/flowdata.%04d-%02d-%02d", savePrefix, 
		localtm->tm_year + 1900, localtm->tm_mon + 1, localtm->tm_mday);
    else
    {
	struct tm *newtm;
	time_t yesterday;

	yesterday = time(NULL) - 86400;
	newtm = localtime(&yesterday);

	snprintf(buf, 99, "%s/flowdata.%04d-%02d-%02d", savePrefix, 
		newtm->tm_year + 1900, newtm->tm_mon + 1, newtm->tm_mday);
    }

    if ((fp = fopen(buf, "w")) == NULL)
	Diep("fopen in ExportRecord() error");


    for (i = 0; i < IP_NUM; i++)
    {
	/*
	inet_ntop(PF_INET, (void *) &(hash_table[i].sin_addr), buf, 16);
	fprintf(fp, "%d\t%s\t", i, buf);

	for (j = 0; j < 24; j++)
	    fprintf(fp, "%llu\t%llu\t", hash_table[i].hflow[j][0], hash_table[i].hflow[j][1]);

	fprintf(fp, "%llu\t%llu\t%llu\n", hash_table[i].nflow[0], hash_table[i].nflow[1], hash_table[i].nflow[2]);
	*/
    }

    fclose(fp);
}


int ImportRecord(char *fname)
{
    int i, j;
    int num;
    FILE *fp;
    char buf[17];

    if (access(fname, R_OK) == -1)
	return 0;

    if ((fp = fopen(fname, "r")) == NULL)
	Diep("fopen in ImportRecord() error");

    for (i = 0; i < IP_NUM; i++)
    {
	/*
	inet_ntop(PF_INET, (void *) &(hash_table[i].sin_addr), buf, 16);
	fscanf(fp, "%d%s", &num, buf);

	if (i != num)
	{
	    Warn("Warn: ImportRecord() invalid format");
	    continue;
	}

	hash_table[i].sin_addr.s_addr = inet_addr(buf);

	for (j = 0; j < 24; j++)
	    fscanf(fp, "%llu%llu", &(hash_table[i].hflow[j][0]), &(hash_table[i].hflow[j][1]));

	fscanf(fp, "%llu%llu%llu", &(hash_table[i].nflow[0]), &(hash_table[i].nflow[1]), &(hash_table[i].nflow[2]));
	*/
    }

    fclose(fp);
    return 1;
}

static void Update()
{
    LoadWhitelist(whitelistFile);

    if (fork() == 0)
    {
	ExportRecord(TODAY);
	exit(EXIT_SUCCESS);
    }
}

static void Usage(char *progName)
{
    printf("Usage: %s [-v] [-i listen_ip] [-p netflow_listen_port] [-P flowd_listen_port] [-w whitelist_file] [-s save_data_path]\n", progName);
    exit(EXIT_SUCCESS);
}

static int LoadConfig(char *fname)
{
    FILE *fp;
    char buf[100];
    char *ipPtr, *maskPtr;
    uint nSubnet = 0;
    in_addr_t addr;
    int maskBits;


    if ((fp = fopen(fname, "r")) == NULL)
	Diep("fopen in LoadConfig() error");

    while (fgets(buf, 99, fp) && nSubnet < MAX_SUBNET)
    {
	if ((ipPtr = strtok(buf, "/\t\n ")) == NULL || (maskPtr = strtok(NULL, "/\t\n ")) == NULL)
	    continue;

	if ((addr = inet_addr(ipPtr)) == INADDR_NONE)
	    continue;

	maskBits = (uint) strtoul(maskPtr, (char **) NULL, 10);
	if (maskBits < 1 && maskBits > 32)
	    continue;

	rcvNetList[nSubnet].net = addr;
	rcvNetList[nSubnet].mask = ~0 << (32 - maskBits);
	rcvNetList[nSubnet].maskBits = (uchar) maskBits;
	rcvNetList[nSubnet].ipCount = (rcvNetList[nSubnet].mask ^ 0xffffffff) + 1;
	sumIpCount += rcvNetList[nSubnet].ipCount;
	rcvNetList[nSubnet].mask = htonl(rcvNetList[nSubnet].mask);

	printf("%u: %08x\t%08x\t%u\n", nSubnet, rcvNetList[nSubnet].net, rcvNetList[nSubnet].mask, rcvNetList[nSubnet].ipCount);
	++nSubnet;
    }

    fclose(fp);

    return nSubnet;
}

static int RcvNetListCmp(const void *a, const void *b)
{
    struct subnet *tmpA = (struct subnet *) a;
    struct subnet *tmpB = (struct subnet *) b;

    return (int) tmpA->net - tmpB->net;
}

static int MapTable()
{
    int i;
    uint n = 0;

    qsort(rcvNetList, nSubnet, sizeof(struct subnet), RcvNetListCmp);

    for (i = 0; i < nSubnet; ++i)
    {

	ipTable[n++] = 
    }

    return 1;
}
void Warn(char *msg)
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

    in_addr_t bindIpAddr;
    uint16_t netflowBindPort;
    uint16_t flowdBindPort;

    int nev;
    struct kevent chlist[2];
    struct kevent evlist[2];

    verbose = 0;
    sumIpCount = 0;
    strncpy(savePrefix, DEF_SAVE_PREFIX, 99);
    strncpy(subnetFile, DEF_SUBNET_FILE, 99);
    strncpy(whitelistFile, DEF_WHITELIST, 99);

    bindIpAddr = INADDR_ANY;
    netflowBindPort = htons(NETFLOW_LISTEN_PORT);
    flowdBindPort = htons(FLOWD_LISTEN_PORT);

    while ((ch = getopt(argc, argv, "i:p:P:s:vw:")) != -1)
    {
	switch ((char) ch)
	{
	    case 'i':		/* Listen IP */
		bindIpAddr = inet_addr(optarg);
		break;

	    case 'p':		/* Netflow Listen Port */
		n = htons(atoi(optarg));
		if (n > 0 && n < 65535)
		    netflowBindPort = htons(atoi(optarg));
		else
		    netflowBindPort = htons(NETFLOW_LISTEN_PORT);
		break;

	    case 'P':		/* Flowd Listen Port */
		n = htons(atoi(optarg));
		if (n > 0 && n < 65535)
		    flowdBindPort = htons(atoi(optarg));
		else
		    flowdBindPort = htons(FLOWD_LISTEN_PORT);
		break;

	    case 's':		/* Path prefix to store */
		SetSavePrefix(optarg);
		break;

	    case 'v':		/* Verbose mode */
		verbose = 1;
		break;

	    case 'w':
		strncpy(whitelistFile, optarg, 99);
		break;

	    case 'f':
		strncpy(subnetFile, optarg, 99);
		break;

	    case '?':
	    default:
		Usage(argv[0]);
		return -1;
	}
    }

    nSubnet = LoadConfig(subnetFile);
    LoadWhitelist(whitelistFile);

    ipTable = (struct hostflow *) malloc(sizeof(struct hostflow) * sumIpCount);
    if (ipTable == NULL)
    {
	fprintf(stderr, "Failed to allocate memory %d bytes\n", sizeof(struct hostflow) * sumIpCount);
	return -1;
    }
    memset(ipTable, 0, sizeof(struct hostflow) * sumIpCount);

    MapTable();

    exit(0);
    now = time(NULL);
    localtm = localtime(&now);
    preHour = localtm->tm_hour;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, &Exits);
    signal(SIGTERM, &Exits);
    signal(SIGHUP, &Update);

    snprintf(buf, BUFSIZE - 1, "%s/flowdata.%04d-%02d-%02d", savePrefix, 
	    localtm->tm_year + 1900, localtm->tm_mon + 1, localtm->tm_mday);

    ImportRecord(buf);
    setvbuf(stdout, NULL, _IONBF, 0);

    netflowSockFd = BuildUDPSck(bindIpAddr, netflowBindPort);
    flowdSockFd = BuildTCPSck(bindIpAddr, flowdBindPort);

    if ((kq = kqueue()) == -1)
	Diep("kqueue() error");

    EV_SET(&chlist[0], netflowSockFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    EV_SET(&chlist[1], flowdSockFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    plen = sizeof(struct sockaddr_in);

    while (1)
    {
	nev = kevent(kq, chlist, 2, evlist, 2, NULL);

	if (nev < 0 && errno != EINTR)
	    Diep("kevent() error");

	for (i = 0; i < nev; i++) 
	{
	    if (evlist[i].flags & EV_ERROR) 
	    {
		fprintf(stderr, "kevent() EV_ERROR: %s\n", strerror(evlist[i].data));
		exit(EXIT_FAILURE);
	    }

	    if (evlist[i].ident == netflowSockFd)
	    {
		memset(buf, 0, BUFSIZE);
		if ((n = recvfrom(netflowSockFd, buf, BUFSIZE, 0, (struct sockaddr *) &pin, &plen)) == -1)
		{
		    perror("recvfrom() error");
		    continue;
		}

		now = time(NULL);
		localtm = localtime(&now);

		if (preHour == 23 && localtm->tm_hour == 0)
		{
		    ExportRecord(YESTERDAY);
		    memset(hashTable, 0, sizeof(struct hostflow) * sumIpCount);
		    preHour = localtm->tm_hour;
		}
		else
		    preHour = localtm->tm_hour;

		if ((ch = isValidNFP(buf, n)) > 0)
		    PushRecord(buf, ch);
	    }
	    else if (evlist[i].ident == flowdSockFd)
	    {
		if ((peerFd = accept(flowdSockFd, (struct sockaddr *) &pin, &plen)) == -1)
		{
		    Diep("accept() error");
		    continue;
		}

		if (fork() == 0)
		{
		    signal(SIGALRM, &SckExit);
		    alarm(30);

		    n = recv(peerFd, buf, BUFSIZE, 0);
		    parseCmd(buf);
		    exit(EXIT_SUCCESS);
		}
		close(peerFd);
	    }
	}
    }

    close(kq);
    close(netflowSockFd);
    close(flowdSockFd);
    free(hashTable);

    return 0;
}

