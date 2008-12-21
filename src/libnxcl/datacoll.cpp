/* 
** NetXMS - Network Management System
** Client Library
** Copyright (C) 2004, 2005, 2006, 2007, 2008 Victor Kirhenshtein
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: datacoll.cpp
**
**/

#include "libnxcl.h"


//
// Load data collection items list for specified node
// This function is NOT REENTRANT
//

DWORD LIBNXCL_EXPORTABLE NXCOpenNodeDCIList(NXC_SESSION hSession, DWORD dwNodeId, 
                                            NXC_DCI_LIST **ppItemList)
{
   return ((NXCL_Session *)hSession)->OpenNodeDCIList(dwNodeId, ppItemList);
}


//
// Unlock and destroy previously opened node's DCI list
//

DWORD LIBNXCL_EXPORTABLE NXCCloseNodeDCIList(NXC_SESSION hSession, NXC_DCI_LIST *pItemList)
{
   DWORD i, j, dwRetCode = RCC_INVALID_ARGUMENT, dwRqId;
   CSCPMessage msg;

   if (pItemList != NULL)
   {
      dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

      msg.SetCode(CMD_UNLOCK_NODE_DCI_LIST);
      msg.SetId(dwRqId);
      msg.SetVariable(VID_OBJECT_ID, pItemList->dwNodeId);
      ((NXCL_Session *)hSession)->SendMsg(&msg);

      dwRetCode = ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);

      for(i = 0; i < pItemList->dwNumItems; i++)
      {
         for(j = 0; j < pItemList->pItems[i].dwNumSchedules; j++)
            free(pItemList->pItems[i].ppScheduleList[j]);
         safe_free(pItemList->pItems[i].ppScheduleList);
         safe_free(pItemList->pItems[i].pThresholdList);
         safe_free(pItemList->pItems[i].pszFormula);
      }
      safe_free(pItemList->pItems);
      free(pItemList);
   }
   return dwRetCode;
}


//
// Create new data collection item
//

DWORD LIBNXCL_EXPORTABLE NXCCreateNewDCI(NXC_SESSION hSession, NXC_DCI_LIST *pItemList, 
                                         DWORD *pdwItemId)
{
   DWORD dwRetCode, dwRqId;
   CSCPMessage msg, *pResponse;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_CREATE_NEW_DCI);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, pItemList->dwNodeId);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwRetCode = pResponse->GetVariableLong(VID_RCC);
      if (dwRetCode == RCC_SUCCESS)
      {
         *pdwItemId = pResponse->GetVariableLong(VID_DCI_ID);

         // Create new entry in list
         pItemList->pItems = (NXC_DCI *)realloc(pItemList->pItems, 
                                                   sizeof(NXC_DCI) * (pItemList->dwNumItems + 1));
         memset(&pItemList->pItems[pItemList->dwNumItems], 0, sizeof(NXC_DCI));
         pItemList->pItems[pItemList->dwNumItems].dwId = *pdwItemId;
         pItemList->pItems[pItemList->dwNumItems].iStatus = ITEM_STATUS_ACTIVE;
         pItemList->pItems[pItemList->dwNumItems].iPollingInterval = 60;
         pItemList->pItems[pItemList->dwNumItems].iRetentionTime = 30;
         pItemList->pItems[pItemList->dwNumItems].iDeltaCalculation = DCM_ORIGINAL_VALUE;
         pItemList->pItems[pItemList->dwNumItems].iAdvSchedule = 0;
         pItemList->pItems[pItemList->dwNumItems].dwNumSchedules = 0;
         pItemList->pItems[pItemList->dwNumItems].ppScheduleList = NULL;
         pItemList->pItems[pItemList->dwNumItems].iProcessAllThresholds = 0;
         pItemList->dwNumItems++;
      }
      delete pResponse;
   }
   else
   {
      dwRetCode = RCC_TIMEOUT;
   }

   return dwRetCode;
}


//
// Update data collection item
//

DWORD LIBNXCL_EXPORTABLE NXCUpdateDCI(NXC_SESSION hSession, DWORD dwNodeId, NXC_DCI *pItem)
{
   DWORD i, dwId, dwRqId, dwRetCode;
   CSCPMessage msg, *pResponse;
   DCI_THRESHOLD dct;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_MODIFY_NODE_DCI);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_ID, pItem->dwId);
   msg.SetVariable(VID_DCI_DATA_TYPE, (WORD)pItem->iDataType);
   msg.SetVariable(VID_POLLING_INTERVAL, (DWORD)pItem->iPollingInterval);
   msg.SetVariable(VID_RETENTION_TIME, (DWORD)pItem->iRetentionTime);
   msg.SetVariable(VID_DCI_SOURCE_TYPE, (WORD)pItem->iSource);
   msg.SetVariable(VID_DCI_DELTA_CALCULATION, (WORD)pItem->iDeltaCalculation);
   msg.SetVariable(VID_DCI_STATUS, (WORD)pItem->iStatus);
   msg.SetVariable(VID_NAME, pItem->szName);
   msg.SetVariable(VID_DESCRIPTION, pItem->szDescription);
   msg.SetVariable(VID_INSTANCE, pItem->szInstance);
   msg.SetVariable(VID_DCI_FORMULA, CHECK_NULL_EX(pItem->pszFormula));
   msg.SetVariable(VID_ALL_THRESHOLDS, (WORD)pItem->iProcessAllThresholds);
   msg.SetVariable(VID_ADV_SCHEDULE, (WORD)pItem->iAdvSchedule);
	msg.SetVariable(VID_RESOURCE_ID, pItem->dwResourceId);
	msg.SetVariable(VID_PROXY_NODE, pItem->dwProxyNode);
   if (pItem->iAdvSchedule)
   {
      msg.SetVariable(VID_NUM_SCHEDULES, pItem->dwNumSchedules);
      for(i = 0, dwId = VID_DCI_SCHEDULE_BASE; i < pItem->dwNumSchedules; i++, dwId++)
         msg.SetVariable(dwId, pItem->ppScheduleList[i]);
   }
   else
   {
      msg.SetVariable(VID_NUM_SCHEDULES, (DWORD)0);
   }
   msg.SetVariable(VID_NUM_THRESHOLDS, pItem->dwNumThresholds);
   for(i = 0, dwId = VID_DCI_THRESHOLD_BASE; i < pItem->dwNumThresholds; i++, dwId++)
   {
      dct.dwId = htonl(pItem->pThresholdList[i].dwId);
      dct.dwEvent = htonl(pItem->pThresholdList[i].dwEvent);
      dct.dwRearmEvent = htonl(pItem->pThresholdList[i].dwRearmEvent);
      dct.dwArg1 = htonl(pItem->pThresholdList[i].dwArg1);
      dct.dwArg2 = htonl(pItem->pThresholdList[i].dwArg2);
      dct.wFunction = htons(pItem->pThresholdList[i].wFunction);
      dct.wOperation = htons(pItem->pThresholdList[i].wOperation);
		dct.nRepeatInterval = htonl(pItem->pThresholdList[i].nRepeatInterval);
      switch(pItem->iDataType)
      {
         case DCI_DT_INT:
         case DCI_DT_UINT:
            dct.value.dwInt32 = htonl(pItem->pThresholdList[i].value.dwInt32);
            break;
         case DCI_DT_INT64:
         case DCI_DT_UINT64:
            dct.value.qwInt64 = htonq(pItem->pThresholdList[i].value.qwInt64);
            break;
         case DCI_DT_FLOAT:
            dct.value.dFloat = htond(pItem->pThresholdList[i].value.dFloat);
            break;
         case DCI_DT_STRING:
#ifdef UNICODE
#ifdef UNICODE_UCS4
				ucs4_to_ucs2(pItem->pThresholdList[i].value.szString, -1, dct.value.szString, MAX_DCI_STRING_VALUE);
#else
            wcscpy(dct.value.szString, pItem->pThresholdList[i].value.szString);
#endif            
#else
				mb_to_ucs2(pItem->pThresholdList[i].value.szString, -1, dct.value.szString, MAX_DCI_STRING_VALUE);
#endif
            SwapWideString(dct.value.szString);
            break;
         default:
            break;
      }
      msg.SetVariable(dwId, (BYTE *)&dct, sizeof(DCI_THRESHOLD));
   }
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwRetCode = pResponse->GetVariableLong(VID_RCC);
      if (dwRetCode == RCC_SUCCESS)
      {
         DWORD dwNumMaps, *pdwMapId, *pdwMapIndex;

         // Get index to id mapping for newly created thresholds
         dwNumMaps = pResponse->GetVariableLong(VID_DCI_NUM_MAPS);
         if (dwNumMaps > 0)
         {
            pdwMapId = (DWORD *)malloc(sizeof(DWORD) * dwNumMaps);
            pdwMapIndex = (DWORD *)malloc(sizeof(DWORD) * dwNumMaps);
            pResponse->GetVariableBinary(VID_DCI_MAP_IDS, (BYTE *)pdwMapId, sizeof(DWORD) * dwNumMaps);
            pResponse->GetVariableBinary(VID_DCI_MAP_INDEXES, (BYTE *)pdwMapIndex, sizeof(DWORD) * dwNumMaps);
            for(i = 0; i < dwNumMaps; i++)
               pItem->pThresholdList[ntohl(pdwMapIndex[i])].dwId = ntohl(pdwMapId[i]);
            free(pdwMapId);
            free(pdwMapIndex);
         }
      }
      delete pResponse;
   }
   else
   {
      dwRetCode = RCC_TIMEOUT;
   }

   return dwRetCode;
}


//
// Delete data collection item
//

DWORD LIBNXCL_EXPORTABLE NXCDeleteDCI(NXC_SESSION hSession, NXC_DCI_LIST *pItemList, 
                                      DWORD dwItemId)
{
   DWORD i, j, dwRqId, dwResult = RCC_INVALID_DCI_ID;
   CSCPMessage msg;

   // Find item with given ID in list
   for(i = 0; i < pItemList->dwNumItems; i++)
      if (pItemList->pItems[i].dwId == dwItemId)
      {
         dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

         msg.SetCode(CMD_DELETE_NODE_DCI);
         msg.SetId(dwRqId);
         msg.SetVariable(VID_OBJECT_ID, pItemList->dwNodeId);
         msg.SetVariable(VID_DCI_ID, dwItemId);
         ((NXCL_Session *)hSession)->SendMsg(&msg);

         dwResult = ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
         if (dwResult == RCC_SUCCESS)
         {
            // Item was successfully deleted on server, delete it from our list
            for(j = 0; j < pItemList->pItems[i].dwNumSchedules; j++)
               free(pItemList->pItems[i].ppScheduleList[j]);
            safe_free(pItemList->pItems[i].ppScheduleList);
            safe_free(pItemList->pItems[i].pThresholdList);
            safe_free(pItemList->pItems[i].pszFormula);
            pItemList->dwNumItems--;
            memmove(&pItemList->pItems[i], &pItemList->pItems[i + 1], 
                    sizeof(NXC_DCI) * (pItemList->dwNumItems - i));
         }
         break;
      }
   return dwResult;
}


//
// Set status for multiple DCIs
//

DWORD LIBNXCL_EXPORTABLE NXCSetDCIStatus(NXC_SESSION hSession, DWORD dwNodeId, DWORD dwNumItems, 
                                         DWORD *pdwItemList, int iStatus)
{
   CSCPMessage msg;
   DWORD dwRqId;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_SET_DCI_STATUS);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_STATUS, (WORD)iStatus);
   msg.SetVariable(VID_NUM_ITEMS, dwNumItems);
   msg.SetVariableToInt32Array(VID_ITEM_LIST, dwNumItems, pdwItemList);
   ((NXCL_Session *)hSession)->SendMsg(&msg);
   return ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
}


//
// Find item in list by id and return it's index
//

DWORD LIBNXCL_EXPORTABLE NXCItemIndex(NXC_DCI_LIST *pItemList, DWORD dwItemId)
{
   DWORD i;

   for(i = 0; i < pItemList->dwNumItems; i++)
      if (pItemList->pItems[i].dwId == dwItemId)
         return i;
   return INVALID_INDEX;
}


//
// Retrieve collected data from server
//

DWORD LIBNXCL_EXPORTABLE NXCGetDCIData(NXC_SESSION hSession, DWORD dwNodeId, DWORD dwItemId, 
                                       DWORD dwMaxRows, DWORD dwTimeFrom, DWORD dwTimeTo, 
                                       NXC_DCI_DATA **ppData)
{
   CSCPMessage msg;
   DWORD i, dwRqId, dwResult;
   BOOL bRun = TRUE;

   msg.SetCode(CMD_GET_DCI_DATA);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_ID, dwItemId);

   // Allocate memory for results and initialize header
   *ppData = (NXC_DCI_DATA *)malloc(sizeof(NXC_DCI_DATA));
   (*ppData)->dwNumRows = 0;
   (*ppData)->dwNodeId = dwNodeId;
   (*ppData)->dwItemId = dwItemId;
   (*ppData)->pRows = NULL;

   do
   {
      dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

      msg.SetId(dwRqId);
      msg.SetVariable(VID_MAX_ROWS, dwMaxRows);
      msg.SetVariable(VID_TIME_FROM, dwTimeFrom);
      msg.SetVariable(VID_TIME_TO, dwTimeTo);
      ((NXCL_Session *)hSession)->SendMsg(&msg);

      dwResult = ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
      if (dwResult == RCC_SUCCESS)
      {
         CSCP_MESSAGE *pRawMsg;

         // We wait a long time because data message can be quite large
         pRawMsg = ((NXCL_Session *)hSession)->WaitForRawMessage(CMD_DCI_DATA, dwRqId, 60000);
         if (pRawMsg != NULL)
         {
            DCI_DATA_HEADER *pHdr;
            DCI_DATA_ROW *pSrc;
            NXC_DCI_ROW *pDst;
            DWORD dwPrevRowCount, dwRecvRows;
            static WORD m_wRowSize[] = { 8, 8, 12, 12, 260, 12 };

            pHdr = (DCI_DATA_HEADER *)pRawMsg->df;
            dwRecvRows = ntohl(pHdr->dwNumRows);

            // Allocate memory for results
            dwPrevRowCount = (*ppData)->dwNumRows;
            (*ppData)->dwNumRows += dwRecvRows;
            (*ppData)->wDataType = (WORD)ntohl(pHdr->dwDataType);
				if ((*ppData)->wDataType > 5)
					(*ppData)->wDataType = 0;
            (*ppData)->wRowSize = m_wRowSize[(*ppData)->wDataType];
            if (dwRecvRows > 0)
               (*ppData)->pRows = (NXC_DCI_ROW *)realloc((*ppData)->pRows, (*ppData)->dwNumRows * (*ppData)->wRowSize);

            // Convert and copy values from message to rows in result
            pSrc = (DCI_DATA_ROW *)(((char *)pHdr) + sizeof(DCI_DATA_HEADER));
            pDst = (NXC_DCI_ROW *)(((char *)((*ppData)->pRows)) + dwPrevRowCount * (*ppData)->wRowSize);
            for(i = 0; i < dwRecvRows; i++)
            {
               pDst->dwTimeStamp = ntohl(pSrc->dwTimeStamp);
               switch((*ppData)->wDataType)
               {
                  case DCI_DT_INT:
                  case DCI_DT_UINT:
                     pDst->value.dwInt32 = ntohl(pSrc->value.dwInteger);
                     break;
                  case DCI_DT_INT64:
                  case DCI_DT_UINT64:
                     pDst->value.qwInt64 = ntohq(pSrc->value.qwInt64);
                     break;
                  case DCI_DT_FLOAT:
                     pDst->value.dFloat = ntohd(pSrc->value.dFloat);
                     break;
                  case DCI_DT_STRING:
                     SwapWideString(pSrc->value.szString);
#ifdef UNICODE
#ifdef UNICODE_UCS4
                     ucs2_to_ucs4(pSrc->value.szString, -1, pDst->value.szString, MAX_STRING_VALUE);
#else
                     wcscpy(pDst->value.szString, pSrc->value.szString);
#endif                     
#else
                     ucs2_to_mb(pSrc->value.szString, -1, pDst->value.szString, MAX_STRING_VALUE);
#endif
                     break;
               }

               pSrc = (DCI_DATA_ROW *)(((char *)pSrc) + (*ppData)->wRowSize);
               pDst = (NXC_DCI_ROW *)(((char *)pDst) + (*ppData)->wRowSize);
            }

            // Shift boundaries
            if (((dwMaxRows == 0) || (dwMaxRows > MAX_DCI_DATA_RECORDS)) &&
                (dwRecvRows == MAX_DCI_DATA_RECORDS))
            {
               // Shift to last record
               pDst = (NXC_DCI_ROW *)(((char *)pDst) - (*ppData)->wRowSize);
               dwTimeTo = pDst->dwTimeStamp - 1;   // Assume that we have no more than one value per second
               if (dwMaxRows > 0)
                  dwMaxRows -= MAX_DCI_DATA_RECORDS;
            }
            else
            {
               bRun = FALSE;
            }

            // Destroy message
            free(pRawMsg);
         }
         else
         {
            dwResult = RCC_TIMEOUT;
         }
      }
   } while((dwResult == RCC_SUCCESS) && bRun);

   // Destroy already allocated buffer if request was unsuccessful
   if (dwResult != RCC_SUCCESS)
   {
      safe_free((*ppData)->pRows);
      free(*ppData);
   }

   return dwResult;
}


//
// Destroy DCI result set
//

void LIBNXCL_EXPORTABLE NXCDestroyDCIData(NXC_DCI_DATA *pData)
{
   if (pData != NULL)
   {
      safe_free(pData->pRows);
      free(pData);
   }
}


//
// Get pointer to specific row in result set
//

NXC_DCI_ROW LIBNXCL_EXPORTABLE *NXCGetRowPtr(NXC_DCI_DATA *pData, DWORD dwRow)
{
   if (dwRow >= pData->dwNumRows)
      return NULL;

   return (NXC_DCI_ROW *)(((char *)(pData->pRows)) + dwRow * pData->wRowSize);
}


//
// Add threshold to item
//

DWORD LIBNXCL_EXPORTABLE NXCAddThresholdToItem(NXC_DCI *pItem, NXC_DCI_THRESHOLD *pThreshold)
{
   DWORD dwIndex;

   dwIndex = pItem->dwNumThresholds++;
   pItem->pThresholdList = (NXC_DCI_THRESHOLD *)realloc(pItem->pThresholdList,
                                    sizeof(NXC_DCI_THRESHOLD) * pItem->dwNumThresholds);
   memcpy(&pItem->pThresholdList[dwIndex], pThreshold, sizeof(NXC_DCI_THRESHOLD));
   return dwIndex;
}


//
// Delete threshold from item
//

BOOL LIBNXCL_EXPORTABLE NXCDeleteThresholdFromItem(NXC_DCI *pItem, DWORD dwIndex)
{
   BOOL bResult = FALSE;

   if (pItem->dwNumThresholds > dwIndex)
   {
      pItem->dwNumThresholds--;
      if (pItem->dwNumThresholds > 0)
         memmove(&pItem->pThresholdList[dwIndex], &pItem->pThresholdList[dwIndex + 1],
                 sizeof(NXC_DCI_THRESHOLD) * (pItem->dwNumThresholds - dwIndex));
      bResult = TRUE;
   }
   return bResult;
}


//
// Swap two threshold items
//

BOOL LIBNXCL_EXPORTABLE NXCSwapThresholds(NXC_DCI *pItem, DWORD dwIndex1, DWORD dwIndex2)
{
   BOOL bResult = FALSE;
   NXC_DCI_THRESHOLD dct;

   if ((pItem->dwNumThresholds > dwIndex1) &&
       (pItem->dwNumThresholds > dwIndex2) &&
       (dwIndex1 != dwIndex2))
   {
      memcpy(&dct, &pItem->pThresholdList[dwIndex1], sizeof(NXC_DCI_THRESHOLD));
      memcpy(&pItem->pThresholdList[dwIndex1], &pItem->pThresholdList[dwIndex2], sizeof(NXC_DCI_THRESHOLD));
      memcpy(&pItem->pThresholdList[dwIndex2], &dct, sizeof(NXC_DCI_THRESHOLD));
      bResult = TRUE;
   }
   return bResult;
}


//
// Copy data collection items from one node to another
//

DWORD LIBNXCL_EXPORTABLE NXCCopyDCI(NXC_SESSION hSession, DWORD dwSrcNodeId, DWORD dwDstNodeId, 
                                    DWORD dwNumItems, DWORD *pdwItemList, BOOL bMove)
{
   CSCPMessage msg;
   DWORD dwRqId;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_COPY_DCI);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_SOURCE_OBJECT_ID, dwSrcNodeId);
   msg.SetVariable(VID_DESTINATION_OBJECT_ID, dwDstNodeId);
   msg.SetVariable(VID_NUM_ITEMS, dwNumItems);
   msg.SetVariableToInt32Array(VID_ITEM_LIST, dwNumItems, pdwItemList);
   msg.SetVariable(VID_MOVE_FLAG, (WORD)bMove);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   return ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
}


//
// Query value of specific parameter from node
//

DWORD LIBNXCL_EXPORTABLE NXCQueryParameter(NXC_SESSION hSession, DWORD dwNodeId, int iOrigin, 
                                           TCHAR *pszParameter, TCHAR *pszBuffer, 
                                           DWORD dwBufferSize)
{
   CSCPMessage msg, *pResponse;
   DWORD dwRqId, dwResult;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_QUERY_PARAMETER);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_SOURCE_TYPE, (WORD)iOrigin);
   msg.SetVariable(VID_NAME, pszParameter);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId, 120000);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult == RCC_SUCCESS)
         pResponse->GetVariableStr(VID_VALUE, pszBuffer, dwBufferSize);
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
   }
   return dwResult;
}


//
// Get last values for all DCIs of selected node
//

DWORD LIBNXCL_EXPORTABLE NXCGetLastValues(NXC_SESSION hSession, DWORD dwNodeId,
                                          DWORD *pdwNumItems, NXC_DCI_VALUE **ppValueList)
{
   CSCPMessage msg, *pResponse;
   DWORD i, dwId, dwRqId, dwResult;

   *pdwNumItems = 0;
   *ppValueList = NULL;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_GET_LAST_VALUES);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult == RCC_SUCCESS)
      {
         *pdwNumItems = pResponse->GetVariableLong(VID_NUM_ITEMS);
         *ppValueList = (NXC_DCI_VALUE *)malloc(sizeof(NXC_DCI_VALUE) * (*pdwNumItems));
         memset(*ppValueList, 0, sizeof(NXC_DCI_VALUE) * (*pdwNumItems));
         for(i = 0, dwId = VID_DCI_VALUES_BASE; i < *pdwNumItems; i++, dwId +=2)
         {
            (*ppValueList)[i].dwId = pResponse->GetVariableLong(dwId++);
            pResponse->GetVariableStr(dwId++, (*ppValueList)[i].szName, MAX_ITEM_NAME);
            pResponse->GetVariableStr(dwId++, (*ppValueList)[i].szDescription, MAX_DB_STRING);
            (*ppValueList)[i].nSource = (BYTE)pResponse->GetVariableShort(dwId++);
            (*ppValueList)[i].nDataType = (BYTE)pResponse->GetVariableShort(dwId++);
            pResponse->GetVariableStr(dwId++, (*ppValueList)[i].szValue, MAX_DB_STRING);
            (*ppValueList)[i].dwTimestamp = pResponse->GetVariableLong(dwId++);
            (*ppValueList)[i].nStatus = (BYTE)pResponse->GetVariableShort(dwId++);
         }
      }
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
   }
   return dwResult;
}


//
// Apply template to node
//

DWORD LIBNXCL_EXPORTABLE NXCApplyTemplate(NXC_SESSION hSession, DWORD dwTemplateId, DWORD dwNodeId)
{
   DWORD dwRqId;
   CSCPMessage msg;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_APPLY_TEMPLATE);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_SOURCE_OBJECT_ID, dwTemplateId);
   msg.SetVariable(VID_DESTINATION_OBJECT_ID, dwNodeId);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   return ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
}


//
// Resolve DCI names
//

DWORD LIBNXCL_EXPORTABLE NXCResolveDCINames(NXC_SESSION hSession, DWORD dwNumDCI,
                                            INPUT_DCI *pDCIList, TCHAR ***pppszNames)
{
   CSCPMessage msg, *pResponse;
   DWORD i, j, dwId, dwRqId, dwResult, *pdwList;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_RESOLVE_DCI_NAMES);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_NUM_ITEMS, dwNumDCI);

   pdwList = (DWORD *)malloc(sizeof(DWORD) * dwNumDCI * 2);
   for(i = 0, j = dwNumDCI; i < dwNumDCI; i++, j++)
   {
      pdwList[i] = pDCIList[i].dwNodeId;
      pdwList[j] = pDCIList[i].dwId;
   }
   msg.SetVariableToInt32Array(VID_NODE_LIST, dwNumDCI, pdwList);
   msg.SetVariableToInt32Array(VID_DCI_LIST, dwNumDCI, &pdwList[dwNumDCI]);
   free(pdwList);

   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult == RCC_SUCCESS)
      {
         *pppszNames = (TCHAR **)malloc(sizeof(TCHAR *) * dwNumDCI);
         for(i = 0, dwId = VID_DCI_LIST_BASE; i < dwNumDCI; i++)
            (*pppszNames)[i] = pResponse->GetVariableStr(dwId++);
      }
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
   }
   return dwResult;
}


//
// Push DCI data
//

DWORD LIBNXCL_EXPORTABLE NXCPushDCIData(NXC_SESSION hSession, DWORD dwNumItems,
                                        NXC_DCI_PUSH_DATA *pItems, DWORD *pdwIndex)
{
   CSCPMessage msg, *pResponse;
   DWORD i, dwRqId, dwId, dwResult;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_PUSH_DCI_DATA);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_NUM_ITEMS, dwNumItems);

   for(i = 0, dwId = VID_PUSH_DCI_DATA_BASE; i < dwNumItems; i++)
   {
      msg.SetVariable(dwId++, pItems[i].dwNodeId);
      if (pItems[i].dwNodeId == 0)
      {
         msg.SetVariable(dwId++, pItems[i].pszNodeName);
      }

      msg.SetVariable(dwId++, pItems[i].dwId);
      if (pItems[i].dwId == 0)
      {
         msg.SetVariable(dwId++, pItems[i].pszName);
      }

      msg.SetVariable(dwId++, pItems[i].pszValue);
   }

   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult != RCC_SUCCESS)
      {
         *pdwIndex = pResponse->GetVariableLong(VID_FAILED_DCI_INDEX);
      }
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
      *pdwIndex = 0;
   }
   return dwResult;
}


//
// Get DCI info
//

DWORD LIBNXCL_EXPORTABLE NXCGetDCIInfo(NXC_SESSION hSession, DWORD dwNodeId,
                                       DWORD dwItemId, NXC_DCI *pInfo)
{
   CSCPMessage msg, *pResponse;
   DWORD dwRqId, dwResult;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_GET_DCI_INFO);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_ID, dwItemId);

   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult == RCC_SUCCESS)
      {
			memset(pInfo, 0, sizeof(NXC_DCI));
			pInfo->dwId= dwItemId;
			pInfo->dwResourceId = pResponse->GetVariableLong(VID_RESOURCE_ID);
			pInfo->dwTemplateId = pResponse->GetVariableLong(VID_TEMPLATE_ID);
			pInfo->iDataType = (BYTE)pResponse->GetVariableShort(VID_DCI_DATA_TYPE);
			pInfo->iSource = (BYTE)pResponse->GetVariableShort(VID_DCI_SOURCE_TYPE);
			pResponse->GetVariableStr(VID_NAME, pInfo->szName, MAX_ITEM_NAME);
			pResponse->GetVariableStr(VID_DESCRIPTION, pInfo->szDescription, MAX_DB_STRING);
      }
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
   }
   return dwResult;
}


//
// Get list of system DCIs
//

DWORD LIBNXCL_EXPORTABLE NXCGetSystemDCIList(NXC_SESSION hSession, DWORD dwNodeId,
                                             DWORD *pdwNumItems, NXC_SYSTEM_DCI **ppList)
{
   CSCPMessage msg, *pResponse;
   DWORD i, dwId, dwRqId, dwResult;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();
	*ppList = NULL;
	*pdwNumItems = 0;

   msg.SetCode(CMD_GET_SYSTEM_DCI_LIST);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);

   ((NXCL_Session *)hSession)->SendMsg(&msg);

   pResponse = ((NXCL_Session *)hSession)->WaitForMessage(CMD_REQUEST_COMPLETED, dwRqId);
   if (pResponse != NULL)
   {
      dwResult = pResponse->GetVariableLong(VID_RCC);
      if (dwResult == RCC_SUCCESS)
      {
			*pdwNumItems = pResponse->GetVariableLong(VID_NUM_ITEMS);
			*ppList = (NXC_SYSTEM_DCI *)malloc(sizeof(NXC_SYSTEM_DCI) * (*pdwNumItems));
			for(i = 0, dwId = VID_SYSDCI_LIST_BASE; i < *pdwNumItems; i++, dwId += 7)
			{
				(*ppList)[i].dwId = pResponse->GetVariableLong(dwId++);
				pResponse->GetVariableStr(dwId++, (*ppList)[i].szName, MAX_DB_STRING);
				(*ppList)[i].nStatus = pResponse->GetVariableShort(dwId++);
			}
      }
      delete pResponse;
   }
   else
   {
      dwResult = RCC_TIMEOUT;
   }
   return dwResult;
}




//
// Clear collected data for DCI
//

DWORD LIBNXCL_EXPORTABLE NXCClearDCIData(NXC_SESSION hSession, DWORD dwNodeId, DWORD dwItemId)
{
   DWORD dwRqId;
   CSCPMessage msg;

   dwRqId = ((NXCL_Session *)hSession)->CreateRqId();

   msg.SetCode(CMD_CLEAR_DCI_DATA);
   msg.SetId(dwRqId);
   msg.SetVariable(VID_OBJECT_ID, dwNodeId);
   msg.SetVariable(VID_DCI_ID, dwItemId);
   ((NXCL_Session *)hSession)->SendMsg(&msg);

   return ((NXCL_Session *)hSession)->WaitForRCC(dwRqId);
}
