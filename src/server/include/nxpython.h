/*
** NetXMS - Network Management System
** Copyright (C) 2003-2018 Victor Kirhenshtein
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
** File: nxpython.h
**
**/

#ifndef _nxpython_h_
#define _nxpython_h_

#if WITH_PYTHON

#include <Python.h>

/**
 * Wrapper for Python sub-interpreter
 */
class PythonInterpreter
{
private:
   PyThreadState *m_threadState;

   PythonInterpreter(PyThreadState *s);

public:
   ~PythonInterpreter();
   
   bool execute(const char *source);
   
   static PythonInterpreter *create();
};

#else

/**
 * Wrapper for Python sub-interpreter
 */
class PythonInterpreter
{
public:
   static PythonInterpreter *create();
};

#endif

#endif