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
#include "log4cxx/logger.h"

log4cxx::LoggerPtr AtmiBrokerMem::logger(log4cxx::Logger::getLogger("Worker"));


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
	LOG4CXX_DEBUG(logger, (char*) "constructor");
}

AtmiBrokerMem::~AtmiBrokerMem() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	freeAllMemory();
}

char*
AtmiBrokerMem::tpalloc(char* type, char* subtype, long size) {
	if (!type) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - no type");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (!subtype && (strcmp(type, "X_COMMON") == 0 || strcmp(type, "X_C_TYPE") == 0)) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - no subtype");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	LOG4CXX_DEBUG(logger, (char*) "tpalloc - type: %s size: %d" << type << " " << size);
	if (strcmp(type, "X_OCTET") == 0) {
		LOG4CXX_DEBUG(logger, (char*) "tpalloc character array ");
		subtype = (char*) "";
	} else if (strncmp(type, "X_COMMON", 8) == 0 || strncmp(type, "X_C_TYPE", 8) == 0) {
		LOG4CXX_DEBUG(logger, (char*) "tpalloc X_COMMON/X_C_TYPE");
		if (size < 1024)
			size = 1024;
	} else {
		LOG4CXX_ERROR(logger, (char*) "tpalloc DONT YET know type: %s" << type);
		tperrno = TPENOENT;
		return NULL;
	}

	LOG4CXX_DEBUG(logger, (char*) "type: %s subtype: %s" << type << "" << subtype);
	MemoryInfo memoryInfo;
	memoryInfo.memoryPtr = (char*) malloc(size * sizeof(char));
	memoryInfo.size = size;
	memoryInfo.type = (char*) malloc(MAX_TYPE_SIZE);
	memoryInfo.subtype = (char*) malloc(MAX_SUBTYPE_SIZE);
	memset(memoryInfo.type, '\0', MAX_TYPE_SIZE);
	memset(memoryInfo.subtype, '\0', MAX_SUBTYPE_SIZE);
	strncpy(memoryInfo.type, type, MAX_TYPE_SIZE);
	strncpy(memoryInfo.subtype, subtype, MAX_SUBTYPE_SIZE);

	LOG4CXX_DEBUG(logger, (char*) "adding MemoryInfo: %p with type: %s with subtype: %s to vector" << (char*) memoryInfo.memoryPtr << ":" << (char*) memoryInfo.type << ":" << (char*) memoryInfo.subtype);
	memoryInfoVector.push_back(memoryInfo);
	LOG4CXX_DEBUG(logger, (char*) "added MemoryInfo to vector");

	return (char*) memoryInfo.memoryPtr;
}

char* AtmiBrokerMem::tprealloc(char * addr, long size) {
	if (!addr) {
		LOG4CXX_ERROR(logger, (char*) "tprealloc - no buffer");
		tperrno = TPEINVAL;
		return NULL;
	}
	if (size < 0) {
		LOG4CXX_ERROR(logger, (char*) "tprealloc - negative size");
		tperrno = TPEINVAL;
		return NULL;
	}

	LOG4CXX_DEBUG(logger, (char*) "tprealloc - addr: %p size: %d" << addr << ":" << size);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		LOG4CXX_DEBUG(logger, (char*) "next memoryInfo id is: %p" << (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == addr) {
			LOG4CXX_DEBUG(logger, (char*) "found matching memory %p" << (*it).memoryPtr);
			LOG4CXX_DEBUG(logger, (char*) "updating memory ptr %p" << (*it).memoryPtr);

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
			(*it).memoryPtr = memPtr;
			(*it).size = size;
			LOG4CXX_DEBUG(logger, (char*) "updated memory ptr %p" << (*it).memoryPtr);
			return memPtr;
		}
	}

	LOG4CXX_DEBUG(logger, (char*) "tprealloc - not found addr: %p size: %d" << addr << ":" << size);
	tperrno = TPEINVAL;
	LOG4CXX_DEBUG(logger, (char*) "tprealloc - not found addr: %p  failure advised" << addr);
	return NULL;
}

void AtmiBrokerMem::tpfree(char* ptr) {
	if (!ptr || ptr == NULL) {
		return;
	}

	LOG4CXX_DEBUG(logger, (char*) "tpfree - ptr: %p" << ptr);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		LOG4CXX_DEBUG(logger, (char*) "next memoryInfo id is: %p" << (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == ptr) {
			MemoryInfo memoryInfo = (*it);
			LOG4CXX_DEBUG(logger, (char*) "freeing memoryPtr: %p" << (char*) memoryInfo.memoryPtr);
			free(memoryInfo.memoryPtr);
			LOG4CXX_DEBUG(logger, (char*) "freeing type: %p" << (char*) memoryInfo.type);
			free(memoryInfo.type);
			LOG4CXX_DEBUG(logger, (char*) "freeing subtype: %p" << (char*) memoryInfo.subtype);
			free(memoryInfo.subtype);
			LOG4CXX_DEBUG(logger, (char*) "freed memory");

			LOG4CXX_DEBUG(logger, (char*) "removing %p from vector");
			memoryInfoVector.erase(it);
			LOG4CXX_DEBUG(logger, (char*) "removed from vector ");

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

	LOG4CXX_DEBUG(logger, (char*) "tptypes - ptr: %p" << ptr);

	for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it != memoryInfoVector.end(); it++) {
		LOG4CXX_DEBUG(logger, (char*) "next memoryInfo id is: %p" << (char*) (*it).memoryPtr);
		if ((*it).memoryPtr == ptr) {
			MemoryInfo memoryInfo = (*it);
			LOG4CXX_DEBUG(logger, (char*) "found matching memory %p" << memoryInfo.memoryPtr);
			LOG4CXX_DEBUG(logger, (char*) "type is %s" << (char*) memoryInfo.type);
			LOG4CXX_DEBUG(logger, (char*) "subtype is %s" << (char*) memoryInfo.subtype);

			if (type) {
				memcpy(type, memoryInfo.type, MAX_TYPE_SIZE);
			}
			if (subtype) {
				memcpy(subtype, memoryInfo.subtype, MAX_SUBTYPE_SIZE);
			}
			return memoryInfo.size;
		}
	}
	// WAS NOT FOUND
	tperrno = TPEINVAL;
	return -1L;
}

void AtmiBrokerMem::freeAllMemory() {
	LOG4CXX_DEBUG(logger, (char*) "freeAllMemory");
	LOG4CXX_DEBUG(logger, (char*) "memoryInfoVector.size %d" << memoryInfoVector.size());
	std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin();
	while (it != memoryInfoVector.end()) {
		MemoryInfo memoryInfo = (*it);
		LOG4CXX_DEBUG(logger, (char*) "freeing memoryPtr: %p" << (char*) memoryInfo.memoryPtr);
		free(memoryInfo.memoryPtr);
		LOG4CXX_DEBUG(logger, (char*) "freeing type: %p" << (char*) memoryInfo.type);
		free(memoryInfo.type);
		LOG4CXX_DEBUG(logger, (char*) "freeing subtype: %p" << (char*) memoryInfo.subtype);
		free(memoryInfo.subtype);
		LOG4CXX_DEBUG(logger, (char*) "freed memory");

		LOG4CXX_DEBUG(logger, (char*) "removing %p from vector");
		it = memoryInfoVector.erase(it);
		LOG4CXX_DEBUG(logger, (char*) "removed from vector ");
	}
	memoryInfoVector.clear();
}
