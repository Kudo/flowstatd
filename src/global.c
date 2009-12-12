#include "flowd.h"

/*
 * Global variables
 */
int verbose;
struct tm localtm;
struct hostflow *ipTable;
struct hostflow *hashTable;
struct subnet myNet;
struct subnet rcvNetList[MAX_SUBNET];
in_addr_t whitelist[MAX_WHITELIST];
uint nSubnet;
uint sumIpCount;
