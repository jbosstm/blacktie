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

#include "xatmi.h"
#include "AtmiBrokerMem.h"
#include "userlog.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerAtmiBrokerMem(log4cxx::Logger::getLogger("AtmiBrokerMem"));

AtmiBrokerMem * AtmiBrokerMem::ptrAtmiBrokerMem = NULL;

AtmiBrokerMem *
AtmiBrokerMem::get_instance() {
	if (ptrAtmiBrokerMem == NULL)
		ptrAtmiBrokerMem = new AtmiBrokerMem();
	return ptrAtmiBrokerMem;
}

void AtmiBrokerMem::discard_instance() {
	if (ptrAtmiBrokerMem != NULL) {
		delete ptrAtmiBrokerMem;
		ptrAtmiBrokerMem = NULL;
	}
}

AtmiBrokerMem::AtmiBrokerMem() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "constructor");
}

AtmiBrokerMem::~AtmiBrokerMem() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "destructor");
	freeAllMemory();
}

char*
AtmiBrokerMem::tpalloc(char* type, char* subtype, long size) {
	if (!type) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - no type");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (!subtype && (strcmp(type, "X_COMMON") == 0 || strcmp(type, "X_C_TYPE") == 0)) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - no subtype");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc - type: %s size: %d", type, size);
	if (strcmp(type, "X_OCTET") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc character array ");
		subtype = (char*) "";
	} else if (strncmp(type, "X_COMMON", 8) == 0 || strncmp(type, "X_C_TYPE", 8) == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpalloc X_COMMON/X_C_TYPE");
		if (size < 1024)
			size = 1024;
	} else {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tpalloc DONT YET know type: %s", type);
		tperrno = TPENOENT;
		return NULL;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "type: %s subtype: %s", type, subtype);
	MemoryInfo memoryInfo;
	memoryInfo.memoryPtr = (char*) malloc(size * sizeof(char));
	memoryInfo.size = size;
	memoryInfo.type = strndup(type, MAX_TYPE_SIZE);
	memoryInfo.subtype = strndup(subtype, MAX_SUBTYPE_SIZE);

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "adding MemoryInfo with type: %s with subtype: %s to vector", (char*) memoryInfo.type, (char*) memoryInfo.subtype);
	memoryInfoVector.push_back(memoryInfo);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "added MemoryInfo to vector");

	return (char*) memoryInfo.memoryPtr;
}

char* AtmiBrokerMem::tprealloc(char * addr, long size) {
	if (!addr) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tprealloc - no buffer");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerMem, (char*) "tprealloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tprealloc - addr: %p size: %d", addr, size);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == addr) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", (*it).memoryPtr);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "updating memory ptr %p", (*it).memoryPtr);

			bool trailingNull = false;
			long reallocSize = size;
			if (strcmp((*it).type, "X_OCTET") == 0) {
				reallocSize = (size + 1) * sizeof(char);
				trailingNull = true;
			} else if (strncmp((*it).type, "X_COMMON", 8) == 0 || strncmp((*it).type, "X_C_TYPE", 8) == 0) {
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
			(*it).memoryPtr = memPtr;
			(*it).size = size;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "updated memory ptr %p", (*it).memoryPtr);
			return memPtr;
		}
	}

	tperrno = TPEINVAL;
	return NULL;
}

void AtmiBrokerMem::tpfree(char* ptr) {
	if (!ptr || ptr == NULL) {
		return;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tpfree - ptr: %p", ptr);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == ptr) {
			MemoryInfo memoryInfo = (*it);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", memoryInfo.memoryPtr);

			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeing memory");
			free(memoryInfo.memoryPtr);
			free(memoryInfo.type);
			free(memoryInfo.subtype);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "freed memory");

			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "removing %p from vector", memoryInfo);
			memoryInfoVector.erase(it);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "removed from vector ");

			break;
		}
	}
	return;
}

long AtmiBrokerMem::tptypes(char* ptr, char* type, char* subtype) {
	if (!ptr || ptr == NULL) {
		tperrno = TPEINVAL;
		return -1;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "tptypes - ptr: %p", ptr);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == ptr) {
			MemoryInfo memoryInfo = (*it);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "found matching memory %p", memoryInfo.memoryPtr);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "type is %s", (char*) memoryInfo.type);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "subtype is %s", (char*) memoryInfo.subtype);

			if (type) {
				strcpy(type, memoryInfo.type);
			}
			if (subtype) {
				strcpy(subtype, memoryInfo.subtype);
			}
			return memoryInfo.size;
		}
	}
	// WAS NOT FOUND
	tperrno = TPEINVAL;
	return -1L;
}

void AtmiBrokerMem::freeAllMemory() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeAllMemory");
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "memoryInfoVector.size %d", memoryInfoVector.size());
	std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin();
	while (it != memoryInfoVector.end()) {
		MemoryInfo memoryInfo = (*it);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "next memoryInfo id is: %p", (char*) memoryInfo.memoryPtr);

		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "freeing memory");
		free(memoryInfo.memoryPtr);
		free(memoryInfo.type);
		free(memoryInfo.subtype);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "freed memory");

		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "removing %p from vector", memoryInfo);
		it = memoryInfoVector.erase(it);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerMem, (char*) "removed from vector ");
	}
	memoryInfoVector.clear();
}
