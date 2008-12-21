/* 
** NetXMS - Network Management System
** Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 Victor Kirhenshtein
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
** File: syncer.cpp
**
**/

#include "nxcore.h"


//
// Externals
//

void NetObjDelete(NetObj *pObject);


//
// Save objects to database
//

void SaveObjects(DB_HANDLE hdb)
{
   DWORD i;
   NetObj *pObject;

   // Delete objects marked for deletion
   RWLockWriteLock(g_rwlockIdIndex, INFINITE);
delete_loop_start:;
   for(i = 0; i < g_dwIdIndexSize; i++)
   {
   	pObject = (NetObj *)g_pIndexById[i].pObject;
      if (pObject->IsDeleted())
      {
         if (pObject->RefCount() == 0)
         {
            DbgPrintf(4, _T("* Syncer * Object %d \"%s\" deleted"), pObject->Id(), pObject->Name());
            pObject->DeleteFromDB();
            NetObjDelete(pObject);
            goto delete_loop_start;   // Restart loop
         }
         else
         {
            DbgPrintf(3, "* Syncer * Unable to delete object with id %d because it is being referenced %d time(s)",
                      pObject->Id(), pObject->RefCount());
         }
      }
   }
   RWLockUnlock(g_rwlockIdIndex);

   // Save objects
   RWLockReadLock(g_rwlockIdIndex, INFINITE);
   DBBegin(hdb);
   for(i = 0; i < g_dwIdIndexSize; i++)
      if (((NetObj *)g_pIndexById[i].pObject)->IsModified())
         ((NetObj *)g_pIndexById[i].pObject)->SaveToDB(hdb);
   DBCommit(hdb);
   RWLockUnlock(g_rwlockIdIndex);
}


//
// Syncer thread
//

THREAD_RESULT THREAD_CALL Syncer(void *arg)
{
   int iSyncInterval;
   DWORD dwWatchdogId;
   DB_HANDLE hdb;

   // Establish separate connection to database if needed
   if (g_dwFlags & AF_ENABLE_MULTIPLE_DB_CONN)
   {
      hdb = DBConnect();
      if (hdb == NULL)
      {
         nxlog_write(MSG_DB_CONNFAIL, EVENTLOG_ERROR_TYPE, NULL);
         hdb = g_hCoreDB;   // Switch to main DB connection
      }
   }
   else
   {
      hdb = g_hCoreDB;
   }

   // Read configuration
   iSyncInterval = ConfigReadInt("SyncInterval", 60);
   DbgPrintf(1, "Syncer thread started, sync_interval = %d", iSyncInterval);
   dwWatchdogId = WatchdogAddThread("Syncer Thread", iSyncInterval * 2 + 10);

   // Main syncer loop
   while(!ShutdownInProgress())
   {
      if (SleepAndCheckForShutdown(iSyncInterval))
         break;   // Shutdown time has arrived
      WatchdogNotify(dwWatchdogId);
      if (!(g_dwFlags & AF_DB_CONNECTION_LOST))    // Don't try to save if DB connection is lost
      {
         SaveObjects(hdb);
         SaveUsers(hdb);
      }
   }

   // Disconnect from database if using separate connection
   if (hdb != g_hCoreDB)
   {
      DBDisconnect(hdb);
   }

   DbgPrintf(1, "Syncer thread terminated");
   return THREAD_OK;
}
