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

#ifndef _FLOWSTATD_H_
#define _FLOWSTATD_H_

#include <sys/types.h>
#include <netinet/in.h>

#define FLOWSTATD_VERSION_MAJOR	    1
#define FLOWSTATD_VERSION_MINOR	    1
#define FLOWSTATD_VERSION_BUILD	    0

#define TRUE		1
#define FALSE		0

//#define	MBYTES		1048576
#define MBYTES		1000000
#define	BUFSIZE		8192

#define	MAX_WHITELIST	50
#define MAX_SUBNET	50

#define	UPLOAD		0
#define	DOWNLOAD	1
#define	SUM		2

#define	DEF_CONFIG_FILE		"/etc/config.json"

#define	TODAY		0
#define	YESTERDAY	1

/*
 * Feature Sets
 */
#undef USE_KQUEUE	    // Only avaliable on FreeBSD or use select() in default

/*
 * Data Struecture
 */
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned int BOOL;

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
 * Global variables
 */
extern int daemonMode;
extern struct tm localtm;
extern struct hostflow *ipTable;
extern struct hostflow *hashTable;
extern struct subnet myNet;
extern struct subnet rcvNetList[MAX_SUBNET];
extern in_addr_t whitelist[MAX_WHITELIST];
extern uint nSubnet;
extern uint sumIpCount;


int ImportRecord(char *fname);
void ExportRecord(int mode);
void Diep(const char *s);
#endif
