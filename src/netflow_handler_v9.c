/*
    flowd - Netflow statistics daemon
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
#include "netflow_handler_v9.h"

static struct NF_V9_template_table_entry *g_templateTable = NULL;

int NfHandlerInitV9Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    g_templateTable = (struct NF_V9_template_table_entry *) malloc(sizeof(struct NF_V9_template_table_entry) * NF_V9_MAX_TEMPLATES);
    memset(g_templateTable, 0, sizeof(struct NF_V9_template_table_entry) * NF_V9_MAX_TEMPLATES);
    return 1;
}

int NfHandlerUnInitV9Impl(NetflowHandlerFunc_t *this)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    if (g_templateTable != NULL) { free(g_templateTable); g_templateTable = NULL; }
    return 1;
}

static int _HashingTemplateId(int templateId)
{
    // Simple hasing here, we don't support templateId large than NF_V9_MAX_TEMPLATES currently.
    return (templateId - 256) % NF_V9_MAX_TEMPLATES;
}

static int _SaveTemplate(struct NF_V9_template_header *templateHeader)
{
    int templateId = _HashingTemplateId(ntohs(templateHeader->template_id));
    int fieldsCount = ntohs(templateHeader->field_count);
    int currField = 0;
    g_templateTable[templateId].template_type = NF_V9_TEMPLATE_TYPE_TEMPLATE;
    g_templateTable[templateId].field_count = fieldsCount;
    for (currField = 0; currField < fieldsCount; ++currField)
    {
	struct NF_V9_flowset_record *flowSetRecord = (struct NF_V9_flowset_record *) (((char *)templateHeader) + sizeof(struct NF_V9_template_header) + currField * sizeof(struct NF_V9_flowset_record));
	g_templateTable[templateId].fields[currField].type = ntohs(flowSetRecord->type);
	g_templateTable[templateId].fields[currField].length = ntohs(flowSetRecord->length);
    }
    return 1;
}

static int _HandleData(struct NF_V9_header *nfHeader, const char *dataBegin, int dataLen)
{
    struct NF_V9_flowset_header *flowSetHeader = (struct NF_V9_flowset_header *) (((char *) nfHeader) + sizeof(struct NF_V9_header));
    int templateId = _HashingTemplateId(ntohs(flowSetHeader->flowset_id));
    int currPos = 0;
    int currField = 0;
    int fieldCount = g_templateTable[templateId].field_count;
    in_addr_t srcAddr = 0;
    in_addr_t dstAddr = 0;
    unsigned int flowBytes = 0;
    struct NF_V9_flowset_record *fields = g_templateTable[templateId].fields;

    NfTimeInfo_t nfTimeInfo;
    nfTimeInfo.SysUptime = ntohl(nfHeader->SysUptime);
    nfTimeInfo.UnixSecs = ntohl(nfHeader->unix_secs);
    nfTimeInfo.UnixNsecs = 0;

    if (g_templateTable[templateId].template_type == NF_V9_TEMPLATE_TYPE_NONE)
    {
	printf("No template data\n");
	return 0;
    }
    while (currField < fieldCount && currPos < dataLen)
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
	    case NF_V9_FIELD_TYPE_IN_BYTES:
		{
		    if (fields[currField].length == 4)
			flowBytes = ntohl(*((uint32_t *)(dataBegin + currPos)));
		    else if (fields[currField].length == 2)
			flowBytes = ntohs(*((uint16_t *)(dataBegin + currPos)));
		    else
			printf("********** byteLength[%d] ***********\n", fields[currField].length);
		}
		break;
	    case NF_V9_FIELD_TYPE_FIRST_SWITCHED:
		{
		    nfTimeInfo.FirstPacketTime = ntohl(*((uint32_t *)(dataBegin + currPos)));
		}
		break;
	}
	++currField;
	currPos += fields[currField].length;
    }

    if (srcAddr != 0 && dstAddr != 0 && flowBytes != 0 && nfTimeInfo.FirstPacketTime != 0)
    {
	InsertNfInfoToIpTable(srcAddr, dstAddr, flowBytes, &nfTimeInfo);
    }
    /*
    else
    {
	printf("------------------- No enough info. ----------------------\n");
    }
    */

    return 1;
}

static void _InsertFlowEntry(const char *packetBuf, int packetLen)
{
    unsigned int flowSetPos = sizeof(struct NF_V9_header);

    while (flowSetPos < packetLen)
    {
	struct NF_V9_flowset_header *flowSetHeader = (struct NF_V9_flowset_header *) (packetBuf + flowSetPos);
	int flowSetId = ntohs(flowSetHeader->flowset_id);
	if (flowSetId == 0)
	{
	    // Template
	    struct NF_V9_template_header *templateHeader = (struct NF_V9_template_header *) (((char *) flowSetHeader) + sizeof(struct NF_V9_flowset_header));
	    _SaveTemplate(templateHeader);
	} else if (flowSetId == 1)
	{
	    // Options Template
	    // TODO: Add if need
	} else if (flowSetId > 255)
	{
	    // Data
	    const char *dataBegin = (const char *) (((char *) flowSetHeader) + sizeof(struct NF_V9_flowset_header));
	    _HandleData((struct NF_V9_header *) packetBuf, dataBegin, ntohs(flowSetHeader->length));
	}
	flowSetPos += ntohs(flowSetHeader->length);
    }
}

int AddFlowDataV9Impl(NetflowHandlerFunc_t *this, const char *packetBuf, int packetLen)
{
    //NetflowHandlerV9_t *nfHandler = (NetflowHandlerV9_t *)this;
    _InsertFlowEntry(packetBuf, packetLen);
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

