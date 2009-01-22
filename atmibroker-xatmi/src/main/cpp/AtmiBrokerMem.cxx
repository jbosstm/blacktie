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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "ThreadLocalStorage.h"
#include "xatmi.h"
//TODO READD #include "extensions.h"
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerBuffers.h"
#include "AtmiBrokerMemc.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerMem(Logger::getLogger("AtmiBrokerMem"));

AtmiBrokerMem * AtmiBrokerMem::ptrAtmiBrokerMem = NULL;

AtmiBrokerMem *
AtmiBrokerMem::get_instance() {
	if (ptrAtmiBrokerMem == NULL)
		ptrAtmiBrokerMem = new AtmiBrokerMem();
	return ptrAtmiBrokerMem;
}

AtmiBrokerMem::AtmiBrokerMem() {
	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "constructor");
}

AtmiBrokerMem::~AtmiBrokerMem() {
	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "destructor");
	freeAllMemory();
}

char*
AtmiBrokerMem::tpalloc(char* type, char* subtype, long size) {
	if (!type) {
		userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - no type");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (!subtype && (strcmp(type, "X_COMMON") == 0 || strcmp(type, "X_C_TYPE") == 0)) {
		userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - no subtype");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc - type: %s size: %d", type, size);
	char * memPtr;
	if (strcmp(type, "X_OCTET") == 0) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc character array ");
		memPtr = (char*) malloc((size + 1) * sizeof(char));
		subtype = (char*) "";
		memPtr[size] = NULL;
	} else if (strcmp(type, TYPE1) == 0) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc TYPE1 struct ");
		if (size < 1024)
			size = 1024;
		memPtr = (char*) malloc(size * sizeof(char));
		memset(memPtr, '\0', size);
		subtype = (char*) "";
	} else if (strncmp(type, "X_COMMON", 8) == 0 || strncmp(type, "X_C_TYPE", 8) == 0) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc X_COMMON/X_C_TYPE");
		if (size < 1024)
			size = 1024;
		memPtr = (char*) malloc(size * sizeof(char));
	} else {
		if (subtype) {
			userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc DONT YET know type: %s subtype: %s", type, subtype);
		} else {
			userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc DONT YET know type: %s", type);
		}
		tperrno = TPENOENT;
		return NULL;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "type: %s subtype: %s", type, subtype);
	MemoryInfo* aMemoryInfo = (MemoryInfo*) malloc(sizeof(MemoryInfo) * 1);
	aMemoryInfo->memoryPtr = memPtr;
	aMemoryInfo->size = size;
	aMemoryInfo->type = strndup(type, MAX_TYPE_SIZE);
	aMemoryInfo->subtype = strndup(subtype, MAX_SUBTYPE_SIZE);

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "adding aMemoryInfo %p with type: %s with subtype: %s to vector", (void*) aMemoryInfo, (char*) aMemoryInfo->type, (char*) aMemoryInfo->subtype);
	memoryInfoVector.push_back(aMemoryInfo);
	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "added aMemoryInfo to vector");

	return memPtr;
}

char*
AtmiBrokerMem::tprealloc(char * addr, long size) {
	if (!addr) {
		userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tprealloc - no buffer");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		userlog(Level::getError(), loggerAtmiBrokerMem, (char*) "tprealloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tprealloc - addr: %p size: %d", addr, size);

	for (std::vector<MemoryInfo*>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it)->memoryPtr);
		if ((*it)->memoryPtr == addr) {
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", (*it)->memoryPtr);
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "updating memory ptr %p", (*it)->memoryPtr);

			bool trailingNull = false;
			long reallocSize = size;
			if (strcmp((*it)->type, "X_OCTET") == 0) {
				reallocSize = (size + 1) * sizeof(char);
				trailingNull = true;
			} else if (strcmp((*it)->type, TYPE1) == 0 || strncmp((*it)->type, "X_COMMON", 8) == 0 || strncmp((*it)->type, "X_C_TYPE", 8) == 0) {
				if (size < 1024)
					size = 1024;
				reallocSize = size * sizeof(char);
			}

			char* memPtr = (char*) realloc((void*) addr, reallocSize);
			if (trailingNull) {
				memPtr[size] = NULL;
			} else {
				memset(memPtr, '\0', size);
			}
			if ((*it)->memoryPtr != memPtr) {
				// TODO this does not delete the original memory if it is moved?
				userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "deleting old memory ptr %p", (*it)->memoryPtr);
			}
			(*it)->memoryPtr = memPtr;
			(*it)->size = size;
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "updated memory ptr %p", (*it)->memoryPtr);
			return memPtr;
		}
	}

	tperrno = TPEINVAL;
	return NULL;
}

void AtmiBrokerMem::tpfree(char* ptr) {
	AtmiBroker_ServiceImpl *service = (AtmiBroker_ServiceImpl*) getSpecific(SVC_KEY);
	if (service != NULL && service->sameBuffer(ptr)) {
		return;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpfree - ptr: %p", ptr);

	for (std::vector<MemoryInfo*>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it)->memoryPtr);
		if ((*it)->memoryPtr == ptr) {
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", (*it)->memoryPtr);

			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeing memory");
			free((*it)->memoryPtr);
			free((*it)->type);
			free((*it)->subtype);
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "freed memory");

			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeing memory info");
			free((*it));
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "freed memory info");

			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "removing %p from vector", (*it));
			memoryInfoVector.erase(it);
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "removed from vector ");

			return;
		}
	}
	return;
}

long AtmiBrokerMem::tptypes(char* ptr, char* type, char* subtype) {
	if (!ptr || ptr == NULL) {
		tperrno = TPEINVAL;
		return -1;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "tptypes - ptr: %p", ptr);

	for (std::vector<MemoryInfo*>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it)->memoryPtr);
		if ((*it)->memoryPtr == ptr) {
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", (*it)->memoryPtr);
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "type is %s", (char*) (*it)->type);
			userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "subtype is %s", (char*) (*it)->subtype);

			if (type) {
				strncpy(type, (*it)->type, MAX_TYPE_SIZE);
			}
			if (subtype) {
				strncpy(subtype, (*it)->subtype, MAX_SUBTYPE_SIZE);
			}
			return (*it)->size;
		}
	}
	// WAS NOT FOUND
	tperrno = TPEINVAL;
	return -1L;
}

void AtmiBrokerMem::freeAllMemory() {
	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeAllMemory");
	userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "memoryInfoVector.size %d", memoryInfoVector.size());

	for (std::vector<MemoryInfo*>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it)->memoryPtr);
		tpfree((char*) (*it)->memoryPtr);

		if (memoryInfoVector.size() == 0)
			return;
		else
			it = memoryInfoVector.begin();
	}
	memoryInfoVector.clear();
}
