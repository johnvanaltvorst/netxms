/*
** NetXMS multiplatform core agent
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
** File: logmonitoring.cpp
**
**/

#include "nms_core.h"

FileMonitoringList g_monitoringList;

FileMonitoringList::FileMonitoringList()
{
   m_mutex = MutexCreate();
   m_monitoredFiles.setOwner(true);
};

FileMonitoringList::~FileMonitoringList()
{
   MutexDestroy(m_mutex);
};

void FileMonitoringList::addMonitoringFile(MONITORED_FILE *fileForAdd)
{
   lock();
   fileForAdd->session->incRefCount();
   m_monitoredFiles.add(fileForAdd);
   unlock();
};

bool FileMonitoringList::checkDublicate(MONITORED_FILE *fileForAdd)
{
   bool result = false;
   lock();
   for(int i = 0; i < m_monitoredFiles.size(); i++)
   {
      m_monitoredFile = m_monitoredFiles.get(i);
      if(_tcscmp(m_monitoredFile->fileName, fileForAdd->fileName) == 0
         && m_monitoredFile->nodeID == fileForAdd->nodeID
         && m_monitoredFile->session->getUserId() == fileForAdd->session->getUserId())
      {
         result=true;
      }
   }
   unlock();
   return result;
};

ObjectArray<ClientSession>* FileMonitoringList::findClientByFNameAndNodID(const TCHAR *fileName, UINT32 nodeID)
{
   lock();
   ObjectArray<ClientSession> *result = new ObjectArray<ClientSession>;
   for(int i = 0; i < m_monitoredFiles.size(); i++)
   {
      m_monitoredFile = m_monitoredFiles.get(i);
      if(_tcscmp(m_monitoredFile->fileName, fileName) == 0 && m_monitoredFile->nodeID == nodeID)
      {
         result->add(m_monitoredFile->session);
      }
   }
   unlock();
   return result;
};

bool FileMonitoringList::removeMonitoringFile(MONITORED_FILE *fileForRemove)
{
   lock();
   bool deleted = false;
   for(int i = 0; i < m_monitoredFiles.size(); i++)
   {
      m_monitoredFile = m_monitoredFiles.get(i);
      if(_tcscmp(m_monitoredFile->fileName, fileForRemove->fileName) == 0 &&
         m_monitoredFile->nodeID == fileForRemove->nodeID &&
         m_monitoredFile->session == fileForRemove->session)
      {
         fileForRemove->session->decRefCount();
         m_monitoredFiles.remove(i);
         deleted = true;
         break;
      }
   }
   unlock();
   return deleted;
};

void FileMonitoringList::removeDisconectedNode(UINT32 nodeId)
{
   lock();
   bool deleted = false;
   for(int i = 0; i < m_monitoredFiles.size(); i++)
   {
      m_monitoredFile = m_monitoredFiles.get(i);
      if(m_monitoredFile->nodeID == nodeId)
      {
         m_monitoredFile->session->decRefCount();
         m_monitoredFiles.remove(i);
      }
   }
   unlock();
};

void FileMonitoringList::lock()
{
   MutexLock(m_mutex);
};

void FileMonitoringList::unlock()
{
   MutexUnlock(m_mutex);
};
