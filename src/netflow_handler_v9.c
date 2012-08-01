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
#include <arpa/inet.h>
#include <sys/socket.h>
#include "liblogger/liblogger.h"
#include "netflow_handler_v9.h"

static struct NF_V9_template_table_entry *g_templateTable = NULL;
static struct NF_V9_source_table_entry *g_sourceTable = NULL;

int NfHandlerInitV9Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    g_templateTable = (struct NF_V9_template_table_entry *) malloc(sizeof(struct NF_V9_template_table_entry) * NF_V9_MAX_TEMPLATES);
    g_sourceTable = (struct NF_V9_source_table_entry *) malloc(sizeof(struct NF_V9_source_table_entry) * NF_V9_MAX_SOURCE_ENTRIES);
    memset(g_templateTable, 0, sizeof(struct NF_V9_template_table_entry) * NF_V9_MAX_TEMPLATES);
    memset(g_sourceTable, 0, sizeof(struct NF_V9_source_table_entry) * NF_V9_MAX_SOURCE_ENTRIES);
    return 1;
}

int NfHandlerUnInitV9Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    if (g_templateTable != NULL) { free(g_templateTable); g_templateTable = NULL; }
    if (g_sourceTable != NULL) { free(g_sourceTable); g_sourceTable = NULL; }
    return 1;
}

static int _HashingTemplateId(int templateId)
{
    // Simple hasing here, we don't support templateId large than NF_V9_TEMPLATES_PER_SOURCE currently.
    return (templateId - 256) % NF_V9_TEMPLATES_PER_SOURCE;
}

static int _HashingSourceInfo(in_addr_t sourceIp, uint32_t sourceId)
{
    // [0] Cantor pairing function
    unsigned long long key = (sourceIp + sourceId) * (sourceIp + sourceId + 1) / 2 + sourceId;

    // Ref: http://elliottback.com/wp/hashmap-implementation-in-c/
    // [1] Robert Jenkins 32 bit Mix Function
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    // [2] Knuth's Multiplicative Hash
    key = (key >> 3) * 2654435761UL;

    return key % NF_V9_MAX_SOURCE_ENTRIES;
}

static int _SaveTemplate(struct sockaddr_in *sourceAddr, uint32_t sourceId, struct NF_V9_template_header *templateHeader)
{
    int sourceOffset = _HashingSourceInfo(sourceAddr->sin_addr.s_addr, sourceId);
    int templateOffset = _HashingTemplateId(ntohs(templateHeader->template_id));
    int templateId = sourceOffset * NF_V9_TEMPLATES_PER_SOURCE + templateOffset;
    int fieldsCount = ntohs(templateHeader->field_count);
    int currField = 0;

    // [0] Collison check
    if (g_sourceTable[sourceOffset].templatePtr != NULL)
    {
	if (g_sourceTable[sourceOffset].sourceIp != sourceAddr->sin_addr.s_addr ||
	    g_sourceTable[sourceOffset].sourceId != sourceId)
	{
	    LogFatal("##### source collison ##### g_sourceTable[sourceOffset].sourceIp[%d] sourceIp[%d] g_sourceTable[sourceOffset].sourceId[%d] sourceId[%d]",
		    g_sourceTable[sourceOffset].sourceIp,
		    sourceAddr->sin_addr.s_addr,
		    g_sourceTable[sourceOffset].sourceId,
		    sourceId);
	}
    }
    else
    {
	g_sourceTable[sourceOffset].sourceIp = sourceAddr->sin_addr.s_addr;
	g_sourceTable[sourceOffset].sourceId = sourceId;
	g_sourceTable[sourceOffset].templatePtr = &(g_templateTable[templateId]);
    }
    /*
    LogDebug("_SaveTemplate[%d] sourceOffset[%d] templateOffset[%d] fieldsCount[%d]", templateId, sourceOffset, templateOffset, fieldsCount);
    {
        char ip[17];

        inet_ntop(PF_INET, (void *) &sourceAddr->sin_addr, ip, 16);
	LogDebug("_SaveTemplate. sourceIp[%s] sourceId[%d] templateId[%d]", ip, sourceId, ntohs(templateHeader->template_id));
    }
    */
    g_templateTable[templateId].template_type = NF_V9_TEMPLATE_TYPE_TEMPLATE;
    g_templateTable[templateId].field_count = (fieldsCount > NF_V9_MAX_FIELDS_IN_TEMPLATE) ? NF_V9_MAX_FIELDS_IN_TEMPLATE : fieldsCount;
    g_templateTable[templateId].record_length = 0;
    for (currField = 0; currField < g_templateTable[templateId].field_count; ++currField)
    {
	struct NF_V9_flowset_record *flowSetRecord = (struct NF_V9_flowset_record *) (((char *)templateHeader) + sizeof(struct NF_V9_template_header) + currField * sizeof(struct NF_V9_flowset_record));
	g_templateTable[templateId].fields[currField].type = ntohs(flowSetRecord->type);
	g_templateTable[templateId].fields[currField].length = ntohs(flowSetRecord->length);
	g_templateTable[templateId].record_length += g_templateTable[templateId].fields[currField].length;
    }
    return sizeof(struct NF_V9_template_header) + currField * sizeof(struct NF_V9_flowset_record);
}

static int _HandleData(struct sockaddr_in *sourceAddr, uint32_t sourceId, struct NF_V9_header *nfHeader, struct NF_V9_flowset_header *flowSetHeader)
{
    const char *dataBegin = (const char *) (((char *) flowSetHeader) + sizeof(struct NF_V9_flowset_header));
    int dataLen = ntohs(flowSetHeader->length);
    int sourceOffset = _HashingSourceInfo(sourceAddr->sin_addr.s_addr, sourceId);
    int templateOffset = _HashingTemplateId(ntohs(flowSetHeader->flowset_id));
    int templateId = sourceOffset * NF_V9_TEMPLATES_PER_SOURCE + templateOffset;
    int currPos = 0;
    int fieldCount = g_templateTable[templateId].field_count;
    struct NF_V9_flowset_record *fields = g_templateTable[templateId].fields;

    /*
    LogDebug("_UseTemplate[%d] sourceOffset[%d] templateOffset[%d] fieldsCount[%d]", templateId, sourceOffset, templateOffset, fieldCount);
    {
        char ip[17];

        inet_ntop(PF_INET, (void *) &sourceAddr->sin_addr, ip, 16);
	LogDebug("_UseTemplate. sourceIp[%s] sourceId[%d] templateId[%d]", ip, sourceId, ntohs(flowSetHeader->flowset_id));
    }
    */

    if (g_templateTable[templateId].template_type == NF_V9_TEMPLATE_TYPE_NONE)
    {
	LogInfo("No template data.");
	return 0;
    }
    while (currPos + g_templateTable[templateId].record_length < dataLen)
    {
	int currField = 0;
	in_addr_t srcAddr = 0;
	in_addr_t dstAddr = 0;
	unsigned int flowBytes = 0;

	NfTimeInfo_t nfTimeInfo;
	nfTimeInfo.SysUptime = ntohl(nfHeader->SysUptime);
	nfTimeInfo.UnixSecs = ntohl(nfHeader->unix_secs);
	nfTimeInfo.UnixNsecs = 0;

	for (currField = 0; currField < fieldCount; ++currField)
	{
	    switch (fields[currField].type)
	    {
		case NF_V9_FIELD_TYPE_IPV4_SRC_ADDR:
		    {
			srcAddr = *((in_addr_t *) (dataBegin + currPos));
		    }
		    break;
		case NF_V9_FIELD_TYPE_IPV4_DST_ADDR:
		    {
			dstAddr = *((in_addr_t *) (dataBegin + currPos));
		    }
		    break;
		case NF_V9_FIELD_TYPE_IPV6_SRC_ADDR:
		case NF_V9_FIELD_TYPE_IPV6_DST_ADDR:
		    {
		    }
		    break;
		case NF_V9_FIELD_TYPE_IN_BYTES:
		    {
			if (fields[currField].length == 4)
			    flowBytes = ntohl(*((uint32_t *)(dataBegin + currPos)));
			else if (fields[currField].length == 2)
			    flowBytes = ntohs(*((uint16_t *)(dataBegin + currPos)));
			else
			    LogWarn("NF_V9_FIELD_TYPE_IN_BYTES is not 2 or 4 bytes. flowBytes[%d]", fields[currField].length);
		    }
		    break;
		case NF_V9_FIELD_TYPE_FIRST_SWITCHED:
		    {
			nfTimeInfo.FirstPacketTime = ntohl(*((uint32_t *)(dataBegin + currPos)));
		    }
		    break;
	    }
	    currPos += fields[currField].length;
	}

	if (srcAddr != 0 && dstAddr != 0 && flowBytes != 0 && nfTimeInfo.FirstPacketTime != 0)
	{
	    InsertNfInfoToIpTable(srcAddr, dstAddr, flowBytes, &nfTimeInfo);
	}
	else
	{
	    //LogDebug("------------------- No enough info. srcAddr[%x] dstAddr[%x] flowBytes[%d] nfTimeInfo.FirstPacketTime[%d]", srcAddr, dstAddr, flowBytes, nfTimeInfo.FirstPacketTime);
	}
    }

    return 1;
}

static void _InsertFlowEntry(const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr)
{
    unsigned int flowSetPos = sizeof(struct NF_V9_header);
    struct NF_V9_header *flowHeader = (struct NF_V9_header *) (packetBuf);

    while (flowSetPos < packetLen)
    {
	struct NF_V9_flowset_header *flowSetHeader = (struct NF_V9_flowset_header *) (packetBuf + flowSetPos);
	int flowSetId = ntohs(flowSetHeader->flowset_id);
	if (flowSetId == 0)
	{
	    // Template
	    int offset = 0;
	    int flowSetLen = ntohs(flowSetHeader->length) - 4;
	    while (offset < flowSetLen)
	    {
		struct NF_V9_template_header *templateHeader = (struct NF_V9_template_header *) (((char *) flowSetHeader) + sizeof(struct NF_V9_flowset_header) + offset);
		offset += _SaveTemplate(sourceAddr, ntohl(flowHeader->source_id), templateHeader);
	    }
	}
	else if (flowSetId == 1)
	{
	    // Options Template
	    // TODO: Add if need
	}
	else if (flowSetId > 255)
	{
	    // Data
	    _HandleData(sourceAddr, ntohl(flowHeader->source_id), (struct NF_V9_header *) packetBuf, flowSetHeader);
	}
	flowSetPos += ntohs(flowSetHeader->length);
    }
}

int AddFlowDataV9Impl(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen, struct sockaddr_in *sourceAddr)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    _InsertFlowEntry(packetBuf, packetLen, sourceAddr);
    return 1;
}

NetflowHandlerFunc_t *NewNetflowHandlerV9()
{
    NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *) malloc(sizeof(NetflowHandlerV9_t));
    nfHandler->funcs.Init = NfHandlerInitV9Impl;
    nfHandler->funcs.UnInit = NfHandlerUnInitV9Impl;
    nfHandler->funcs.AddFlowData = AddFlowDataV9Impl;
    return &(nfHandler->funcs);
}

int FreeNetflowHandlerV9(NetflowHandlerFunc_t *this)
{
    this->UnInit(this);
    NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    if (nfHandler != NULL)
    {
	free(nfHandler);
	return 1;
    }
    else
    {
	return 0;
    }
}

