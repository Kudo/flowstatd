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
#include <string.h>
#include <time.h>
#include <zlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "jansson.h"
#include "flowstatd.h"
#include "socket.h"
#include "netflow.h"
#include "errorCode.h"
#include "liblogger/liblogger.h"

extern int peerFd;
extern char savePrefix[100];
extern char *secretKey;

static void GetByIP(json_t *jsonResp, const char *ipaddr, BOOL isShowAll)
{
    int i = 0;
    int ipIdx = 0;
    in_addr_t ipaddr_in;
    BOOL isOurNet = FALSE;
    BOOL isWhitelist = FALSE;
    json_t *jsonData = json_array();
    unsigned long long int totalFlow = 0;

    SET_JSON_RET_INFO(jsonResp, E_FAILED);

    for (i = 0; i < (int) nSubnet; ++i) {
	if (((ipaddr_in = inet_addr(ipaddr)) & rcvNetList[i].mask) == rcvNetList[i].net) {
	    isOurNet = TRUE;
	    break;
	}
    }
    if (isOurNet == FALSE) {
	SET_JSON_RET_INFO(jsonResp, E_NO_DATA);
	goto Exit;
    }
    ipIdx = getIPIdx(ipaddr_in);

    // ip
    json_object_set_new(jsonResp, "ip", json_string(ipaddr));

    if (isShowAll == FALSE) {
	int j = 0;
	for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++) {
	    if (ipaddr_in == whitelist[j]) {
		isWhitelist = TRUE;
		break;
	    }
	}
    }

    // total
    if (isWhitelist)
	json_object_set_new(jsonResp, "total", json_real(0));
    else
	json_object_set_new(jsonResp, "total", json_real(((double) ipTable[ipIdx].nflow[SUM]) / MBYTES));

    for (i = 0; i < 24; i++) {
	json_t *jsonDataEntity = json_object();
	totalFlow += ipTable[ipIdx].hflow[i][UPLOAD] + ipTable[ipIdx].hflow[i][DOWNLOAD];

	json_object_set_new(jsonDataEntity, "hour", json_integer(i));
	if (isWhitelist) {
	    json_object_set_new(jsonDataEntity, "upload", json_real(0));
	    json_object_set_new(jsonDataEntity, "download", json_real(0));
	    json_object_set_new(jsonDataEntity, "total", json_real(0));
	    json_object_set_new(jsonDataEntity, "incrementalTotal", json_real(0));
	} else {
	    json_object_set_new(jsonDataEntity, "upload", json_real(((double) ipTable[ipIdx].hflow[i][UPLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "download", json_real(((double) ipTable[ipIdx].hflow[i][DOWNLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "total", json_real(((double) (ipTable[ipIdx].hflow[i][UPLOAD] + ipTable[ipIdx].hflow[i][DOWNLOAD])) / MBYTES));
	    json_object_set_new(jsonDataEntity, "incrementalTotal", json_real(((double) (totalFlow)) / MBYTES));
	}
	json_array_append_new(jsonData, jsonDataEntity);
    }
    json_object_set_new(jsonResp, "data", jsonData);
    jsonData = NULL;
    SET_JSON_RET_INFO(jsonResp, S_OK);

Exit:
    if (jsonData != NULL) { json_decref(jsonData); }
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

static void GetByFlow(json_t *jsonResp, uint overMB, BOOL isShowAll)
{
    int i = 0;
    uint rank = 0;
    json_t *jsonData = json_array();

    SET_JSON_RET_INFO(jsonResp, E_FAILED);

    if (overMB <= 0) {
	SET_JSON_RET_INFO(jsonResp, E_INVALID_PARAM);
	goto Exit;
    }

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    if (jsonData == NULL) {
	LogError("Unable to create a new json data");
	goto Exit;
    }

    // overValue
    json_object_set_new(jsonResp, "overValue", json_integer(overMB));

    i = 0;
    while ((ipTable[i].nflow[SUM] / MBYTES) >= overMB) {
	char ip[17] = {0};
	BOOL isShow = TRUE;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, 16);

	if (isShowAll == FALSE) {
	    int j = 0;
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++) {
		if (ipTable[i].sin_addr.s_addr == whitelist[j]) {
		    isShow = FALSE;
		    break;
		}
	    }
	}

	if (isShow) {
	    json_t *jsonDataEntity = json_object();
	    json_object_set_new(jsonDataEntity, "rank", json_integer(++rank));
	    json_object_set_new(jsonDataEntity, "ip", json_string(ip));
	    json_object_set_new(jsonDataEntity, "upload", json_real(((double) ipTable[i].nflow[UPLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "download", json_real(((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "total", json_real(((double) ipTable[i].nflow[SUM]) / MBYTES));
	    json_array_append_new(jsonData, jsonDataEntity);
	}
	++i;
    }
    json_object_set_new(jsonResp, "data", jsonData);
    jsonData = NULL;
    SET_JSON_RET_INFO(jsonResp, S_OK);

Exit:
    if (jsonData != NULL) { json_decref(jsonData); }
}

static void GetByTopN(json_t *jsonResp, uint topN, BOOL isShowAll)
{
    int i = 0;
    uint rank = 0;
    json_t *jsonData = json_array();

    SET_JSON_RET_INFO(jsonResp, E_FAILED);

    if (topN <= 0 || topN >= sumIpCount) {
	SET_JSON_RET_INFO(jsonResp, E_INVALID_PARAM);
	goto Exit;
    }

    qsort(ipTable, sumIpCount, sizeof(struct hostflow), HostFlowCmp);

    if (jsonData == NULL) {
	LogError("Unable to create a new json data");
	goto Exit;
    }

    // limit
    json_object_set_new(jsonResp, "limit", json_integer(topN));

    // data
    while (rank < topN) {
	char ip[17] = {0};
	BOOL isShow = TRUE;
	inet_ntop(PF_INET, (void *) &(ipTable[i].sin_addr), ip, sizeof(ip) - 1);

	if (isShowAll == FALSE) {
	    int j = 0;
	    for (j = 0; whitelist[j] != 0 && j < MAX_WHITELIST; j++) {
		if (ipTable[i].sin_addr.s_addr == whitelist[j]) {
		    isShow = FALSE;
		    break;
		}
	    }
	}

	if (isShow) {
	    json_t *jsonDataEntity = json_object();
	    json_object_set_new(jsonDataEntity, "rank", json_integer(++rank));
	    json_object_set_new(jsonDataEntity, "ip", json_string(ip));
	    json_object_set_new(jsonDataEntity, "upload", json_real(((double) ipTable[i].nflow[UPLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "download", json_real(((double) ipTable[i].nflow[DOWNLOAD]) / MBYTES));
	    json_object_set_new(jsonDataEntity, "total", json_real(((double) ipTable[i].nflow[SUM]) / MBYTES));
	    json_array_append_new(jsonData, jsonDataEntity);
	}
	++i;
    }
    json_object_set_new(jsonResp, "data", jsonData);
    jsonData = NULL;
    SET_JSON_RET_INFO(jsonResp, S_OK);

Exit:
    if (jsonData != NULL) { json_decref(jsonData); }
}

BOOL parseCmd(const char *_jsonData)
{
    BOOL ret = TRUE;
    const char *cmd = NULL;
    const char *date = NULL;
    char *output = NULL;
    json_t *jsonResp = json_object();
    json_t *jsonRoot = NULL;
    json_t *jsonCmd = NULL;
    json_t *jsonDate = NULL;
    json_t *jsonShowAll = NULL;
    json_error_t jsonError = {0};
    BOOL isToday = FALSE;
    BOOL isShowAll = FALSE;
    struct tm tm = {0};

    SET_JSON_RET_INFO(jsonResp, E_INVALID_COMMAND);

    jsonRoot = json_loads(_jsonData, 0, &jsonError);
    if (jsonRoot == NULL) {
	LogError("Unable to parse command. _jsonData[%s] jsonErr[%s@%d]", _jsonData, jsonError.text, jsonError.line);
	ret = FALSE;
	goto Exit;
    }
    if (!json_is_object(jsonRoot)) {
	LogError("jsonRoot is not an object.");
	ret = FALSE;
	goto Exit;
    }

    // parse command
    jsonCmd = json_object_get(jsonRoot, "command");
    if (jsonCmd == NULL || !json_is_string(jsonCmd)) {
	LogError("Invalid command format.");
	ret = FALSE;
	goto Exit;
    }
    cmd = json_string_value(jsonCmd);
    json_object_set_new(jsonResp, "command", json_string(cmd));

    // parse date
    jsonDate = json_object_get(jsonRoot, "date");
    if (jsonDate == NULL || !json_is_string(jsonDate)) {
	LogError("Invalid date format.");
	ret = FALSE;
	goto Exit;
    }
    date = json_string_value(jsonDate);
    if (strcasecmp(date, "today") == 0) {
	char buf[BUFSIZE] = {0};
	strftime(buf, sizeof(buf) - 1, "%F %T", &localtm);
	json_object_set_new(jsonResp, "datetime", json_string(buf));
	isToday = TRUE;
    } else {
	char buf[BUFSIZE] = {0};
	if (strptime(date, "%Y-%m-%d", &tm) == NULL) {
	    LogError("Invalid date format.");
	    ret = FALSE;
	    goto Exit;
	}
	snprintf(buf, BUFSIZE - 1, "%s/flowstatdata.%04d-%02d-%02d.gz", savePrefix, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	if (ImportRecord(buf) == 0) {
	    SET_JSON_RET_INFO(jsonResp, E_NO_DATA);
	    ret = FALSE;
	    goto Exit;
	}

	snprintf(buf, BUFSIZE - 1, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	json_object_set_new(jsonResp, "date", json_string(buf));
    }

    // parse showAll
    jsonShowAll = json_object_get(jsonRoot, "showAll");
    if (jsonShowAll && json_is_string(jsonShowAll) && strcmp(json_string_value(jsonShowAll), secretKey) == 0) {
	isShowAll = TRUE;
    }

    // getFlow command
    if (strcasecmp(cmd, "getFlow") == 0) {
	json_t *jsonIp = NULL;
	const char *ip = NULL;

	jsonIp = json_object_get(jsonRoot, "ip");
	if (jsonIp == NULL || !json_is_string(jsonIp)) {
	    LogError("Invalid ip format.");
	    ret = FALSE;
	    goto Exit;
	}
	ip = json_string_value(jsonIp);

	GetByIP(jsonResp, ip, isShowAll);
    } 
    // showOverList command
    else if (strcasecmp(cmd, "showOverList") == 0) {
	json_t *jsonOverValue = NULL;
	int overValue = 0;

	jsonOverValue = json_object_get(jsonRoot, "overValue");
	if (jsonOverValue == NULL || !json_is_integer(jsonOverValue)) {
	    LogError("Invalid overValue format.");
	    ret = FALSE;
	    goto Exit;
	}
	overValue = json_integer_value(jsonOverValue);

	GetByFlow(jsonResp, overValue, isShowAll);
    }
    // showTopList Command
    else if (strcasecmp(cmd, "showTopList") == 0) {
	json_t *jsonLimit = NULL;
	int limit = 0;

	jsonLimit = json_object_get(jsonRoot, "limit");
	if (jsonLimit == NULL || !json_is_integer(jsonLimit)) {
	    LogError("Invalid limit format.");
	    ret = FALSE;
	    goto Exit;
	}
	limit = json_integer_value(jsonLimit);

	GetByTopN(jsonResp, limit, isShowAll);
    }
    else {
	LogError("Invalid Command. _jsonData[%s]", _jsonData);
	ret = FALSE;
    }

Exit:
    output = json_dumps(jsonResp, JSON_INDENT(4));
    SendBufToSock(peerFd, output, strlen(output));
    if (output != NULL) { free(output); output = NULL; }

    if (jsonRoot != NULL) { json_decref(jsonRoot); }
    if (jsonResp != NULL) { json_decref(jsonResp); }

    return ret;
}

