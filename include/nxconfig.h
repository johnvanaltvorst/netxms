/* 
** NetXMS - Network Management System
** Copyright (C) 2003-2010 Victor Kirhenshtein
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** File: nxconfig.h
**
**/

#ifndef _nxconfig_h_
#define _nxconfig_h_

#include <nms_common.h>
#include <nms_util.h>
#include <uuid.h>

/**
 * Config entry
 */
class LIBNETXMS_EXPORTABLE ConfigEntry
{
private:
	TCHAR *m_name;
	ConfigEntry *m_parent;
	ConfigEntry *m_next;
	ConfigEntry *m_first;
   ConfigEntry *m_last;
	int m_valueCount;
	TCHAR **m_values;
	TCHAR *m_file;
	int m_line;
	int m_id;
   StringMap m_attributes;

	void addEntry(ConfigEntry *entry);
	void linkEntry(ConfigEntry *entry) { entry->m_next = m_next; m_next = entry; }

public:
	ConfigEntry(const TCHAR *name, ConfigEntry *parent, const TCHAR *file, int line, int id);
	~ConfigEntry();

	ConfigEntry *getNext() { return m_next; }
	ConfigEntry *getParent() { return m_parent; }

	const TCHAR *getName() { return m_name; }
	int getId() { return m_id; }
	int getValueCount() { return m_valueCount; }
	int getConcatenatedValuesLength();
	
	const TCHAR *getValue(int index = 0);
	INT32 getValueInt(int index = 0, INT32 defaultValue = 0);
	UINT32 getValueUInt(int index = 0, UINT32 defaultValue = 0);
	INT64 getValueInt64(int index = 0, INT64 defaultValue = 0);
	UINT64 getValueUInt64(int index = 0, UINT64 defaultValue = 0);
	bool getValueBoolean(int index = 0, bool defaultValue = false);
	bool getValueUUID(int index, uuid_t uuid);

	const TCHAR *getSubEntryValue(const TCHAR *name, int index = 0, const TCHAR *defaultValue = NULL);
	INT32 getSubEntryValueInt(const TCHAR *name, int index = 0, INT32 defaultValue = 0);
	UINT32 getSubEntryValueUInt(const TCHAR *name, int index = 0, UINT32 defaultValue = 0);
	INT64 getSubEntryValueInt64(const TCHAR *name, int index = 0, INT64 defaultValue = 0);
	UINT64 getSubEntryValueUInt64(const TCHAR *name, int index = 0, UINT64 defaultValue = 0);
	bool getSubEntryValueBoolean(const TCHAR *name, int index = 0, bool defaultValue = false);
	bool getSubEntryValueUUID(const TCHAR *name, uuid_t uuid, int index = 0);

   const TCHAR *getAttribute(const TCHAR *name) { return m_attributes.get(name); }
   void setAttribute(const TCHAR *name, const TCHAR *value) { m_attributes.set(name, value); }
   void setAttributePreallocated(TCHAR *name, TCHAR *value) { m_attributes.setPreallocated(name, value); }

	const TCHAR *getFile() { return m_file; }
	int getLine() { return m_line; }

	void setName(const TCHAR *name);

	void addValue(const TCHAR *value);
	void setValue(const TCHAR*value);

	ConfigEntry *createEntry(const TCHAR *name);
	ConfigEntry *findEntry(const TCHAR *name);
	ObjectArray<ConfigEntry> *getSubEntries(const TCHAR *mask);
	ObjectArray<ConfigEntry> *getOrderedSubEntries(const TCHAR *mask);
	void unlinkEntry(ConfigEntry *entry);

	void print(FILE *file, int level);
	void createXml(String &xml, int level = 0);
};

/**
 * Hierarchical config
 */
class LIBNETXMS_EXPORTABLE Config
{
private:
	ConfigEntry *m_root;
	int m_errorCount;
	MUTEX m_mutex;

protected:
	virtual void onError(const TCHAR *errorMessage);
	
	void error(const TCHAR *format, ...);
	ConfigEntry *createEntry(const TCHAR *path);

public:
	Config();
   virtual ~Config();

	void lock() { MutexLock(m_mutex); }
	void unlock() { MutexUnlock(m_mutex); }

	void setTopLevelTag(const TCHAR *topLevelTag) { m_root->setName(topLevelTag); }

	bool loadXmlConfig(const TCHAR *file, const char *topLevelTag = NULL);
	bool loadXmlConfigFromMemory(const char *xml, int xmlSize, const TCHAR *name = NULL, const char *topLevelTag = NULL);
	bool loadIniConfig(const TCHAR *file, const TCHAR *defaultIniSection, bool ignoreErrors = true);
	bool loadConfig(const TCHAR *file, const TCHAR *defaultIniSection, bool ignoreErrors = true);

	bool loadConfigDirectory(const TCHAR *path, const TCHAR *defaultIniSection, bool ignoreErrors = true);

	void deleteEntry(const TCHAR *path);

	ConfigEntry *getEntry(const TCHAR *path);
	const TCHAR *getValue(const TCHAR *path, const TCHAR *defaultValue = NULL);
	INT32 getValueInt(const TCHAR *path, INT32 defaultValue);
	UINT32 getValueUInt(const TCHAR *path, UINT32 defaultValue);
	INT64 getValueInt64(const TCHAR *path, INT64 defaultValue);
	UINT64 getValueUInt64(const TCHAR *path, UINT64 defaultValue);
	bool getValueBoolean(const TCHAR *path, bool defaultValue);
	bool getValueUUID(const TCHAR *path, uuid_t uuid);
	ObjectArray<ConfigEntry> *getSubEntries(const TCHAR *path, const TCHAR *mask);
	ObjectArray<ConfigEntry> *getOrderedSubEntries(const TCHAR *path, const TCHAR *mask);

	bool setValue(const TCHAR *path, const TCHAR *value);
	bool setValue(const TCHAR *path, INT32 value);
	bool setValue(const TCHAR *path, UINT32 value);
	bool setValue(const TCHAR *path, INT64 value);
	bool setValue(const TCHAR *path, UINT64 value);
	bool setValue(const TCHAR *path, double value);
	bool setValue(const TCHAR *path, uuid_t value);

	bool parseTemplate(const TCHAR *section, NX_CFG_TEMPLATE *cfgTemplate);

	int getErrorCount() { return m_errorCount; }

	void print(FILE *file);
	String createXml();
};


#endif
