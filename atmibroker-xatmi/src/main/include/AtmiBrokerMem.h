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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

// AtmiBrokerMem.h

#ifndef ATMIBROKERMEM_H
#define ATMIBROKERMEM_H

#include <iostream>
#include <vector>

#define MAX_TYPE_SIZE 8
#define MAX_SUBTYPE_SIZE 16
struct ATMIBROKER_DLL _memory_info {
	void* memoryPtr;
	char* type;
	char* subtype;
	int size;
};
typedef ATMIBROKER_DLL _memory_info MemoryInfo;

class ATMIBROKER_DLL AtmiBrokerMem
{

public:

	AtmiBrokerMem();

	~AtmiBrokerMem();

	char* tpalloc (char* type, char* subtype, long size);

	char* tprealloc (char * addr, long size);

	void tpfree (char* ptr);

	long tptypes (char* ptr, char* type, char* subtype);

	void freeAllMemory();

	static AtmiBrokerMem* get_instance();
	static void discard_instance();

private:

	std::vector<MemoryInfo> memoryInfoVector;

	static AtmiBrokerMem * ptrAtmiBrokerMem;

};

#endif
