/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include "ThreadLocalStorage.h"

#ifndef WIN32
__thread void * specific = 0;

extern int getKey() {
	return -1;
}
extern bool setSpecific(int key, void* threadData) {
	specific = threadData;
	return true;
}
extern void* getSpecific(int key) {
	return specific;
}

extern bool destroySpecific(int key) {
	specific = 0;
	return true;
}
#else
#include <windows.h>

extern int getKey()
{
	return TlsAlloc();
}

extern bool setSpecific (int key, void* threadData)
{
	if (TlsSetValue(key, (LPVOID) threadData))
	return TRUE;
	return FALSE;
}

extern void* getSpecific (int key)
{
	LPVOID ptr = TlsGetValue(key);
	if (ptr)
	return ptr;
	else
	return NULL;
}

extern bool destroySpecific (int key)
{
	if (TlsSetValue(key, (LPVOID) 0))
	return TRUE;
	return FALSE;
}
#endif
