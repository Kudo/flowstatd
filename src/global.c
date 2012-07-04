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

#include <time.h>
#include "flowstatd.h"

/*
 * Global variables
 */
int verbose;
int daemonMode;
int debug;
struct tm localtm;
struct hostflow *ipTable;
struct hostflow *hashTable;
struct subnet myNet;
struct subnet rcvNetList[MAX_SUBNET];
in_addr_t whitelist[MAX_WHITELIST];
uint nSubnet;
uint sumIpCount;
