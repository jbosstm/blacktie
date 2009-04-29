/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "xatmi.h"
#include "AtmiBrokerMem.h"
#include "log4cxx/logger.h"
#include "ThreadLocalStorage.h"
//#include "AtmiBrokerCodes.h"
char* bTPERESET = (char*) "0";
char* bTPEBADDESC = (char*) "2";
char* bTPEBLOCK = (char*) "3";
char* bTPEINVAL = (char*) "4";
char* bTPELIMIT = (char*) "5";
char* bTPENOENT = (char*) "6";
char* bTPEOS = (char*) "7";
char* bTPEPROTO = (char*) "9";
char* bTPESVCERR = (char*) "10";
char* bTPESVCFAIL = (char*) "11";
char* bTPESYSTEM = (char*) "12";
char* bTPETIME = (char*) "13";
char* bTPETRAN = (char*) "14";
char* bTPGOTSIG = (char*) "15";
char* bTPEITYPE = (char*) "17";
char* bTPEOTYPE = (char*) "18";
char* bTPEEVENT = (char*) "22";
char* bTPEMATCH = (char*) "23";

log4cxx::LoggerPtr AtmiBrokerMem::logger(log4cxx::Logger::getLogger(
		"AtmiBrokerMem"));
SynchronizableObject* AtmiBrokerMem::lock = new SynchronizableObject();

AtmiBrokerMem * AtmiBrokerMem::ptrAtmiBrokerMem = NULL;

AtmiBrokerMem *
AtmiBrokerMem::get_instance() {
	LOG4CXX_TRACE(logger, (char*) "get_instance locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "get_instance locked");
	if (ptrAtmiBrokerMem == NULL)
		ptrAtmiBrokerMem = new AtmiBrokerMem();
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "get_instance unlocked");
	return ptrAtmiBrokerMem;
}

void AtmiBrokerMem::discard_instance() {
	LOG4CXX_TRACE(logger, (char*) "discard_instance locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "discard_instance locked");
	if (ptrAtmiBrokerMem != NULL) {
		delete ptrAtmiBrokerMem;
		ptrAtmiBrokerMem = NULL;
	}
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "discard_instance unlocked");
}

AtmiBrokerMem::AtmiBrokerMem() {
	LOG4CXX_DEBUG(logger, (char*) "constructor");
}

AtmiBrokerMem::~AtmiBrokerMem() {
	LOG4CXX_DEBUG(logger, (char*) "destructor assumes you have the lock....");
	LOG4CXX_DEBUG(logger, (char*) "memoryInfoVector.size %d"
			<< memoryInfoVector.size());
	std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin();
	while (it != memoryInfoVector.end()) {
		MemoryInfo memoryInfo = (*it);
		LOG4CXX_DEBUG(logger, (char*) "freeing memoryPtr: %p"
				<< (char*) memoryInfo.memoryPtr);
		free(memoryInfo.memoryPtr);
		LOG4CXX_DEBUG(logger, (char*) "freeing type: %p"
				<< (char*) memoryInfo.type);
		free(memoryInfo.type);
		LOG4CXX_DEBUG(logger, (char*) "freeing subtype: %p"
				<< (char*) memoryInfo.subtype);
		free(memoryInfo.subtype);
		LOG4CXX_DEBUG(logger, (char*) "freed memory");

		LOG4CXX_DEBUG(logger, (char*) "removing %p from vector");
		it = memoryInfoVector.erase(it);
		LOG4CXX_DEBUG(logger, (char*) "removed from vector ");
	}
	memoryInfoVector.clear();
	LOG4CXX_TRACE(logger, (char*) "freeAllMemory unlocked");
}

char*
AtmiBrokerMem::tpalloc(char* type, char* subtype, long size) {
	char* toReturn = NULL;
	LOG4CXX_TRACE(logger, (char*) "tpalloc locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "tpalloc locked");
	if (!type) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - no type");
		setSpecific(TPE_KEY, bTPEINVAL);
	} else if (!subtype && (strcmp(type, "X_COMMON") == 0 || strcmp(type,
			"X_C_TYPE") == 0)) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - no subtype");
		setSpecific(TPE_KEY, bTPEINVAL);
	} else if (size < 0) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc - negative size");
		setSpecific(TPE_KEY, bTPEINVAL);
	} else if (strncmp(type, "X_OCTET", MAX_TYPE_SIZE) != 0 && strncmp(type,
			"X_COMMON", MAX_TYPE_SIZE) != 0 && strncmp(type, "X_C_TYPE",
			MAX_TYPE_SIZE) != 0) {
		LOG4CXX_ERROR(logger, (char*) "tpalloc DONT YET know type: %s" << type);
		setSpecific(TPE_KEY, bTPENOENT);
	} else {
		if (strcmp(type, "X_OCTET") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "tpalloc character array ");
			subtype = (char*) "";
		} else {
			LOG4CXX_DEBUG(logger, (char*) "tpalloc X_COMMON/X_C_TYPE");
			if (size < 1024)
				size = 1024;
		}
		LOG4CXX_DEBUG(logger, (char*) "tpalloc - type: subtype: size:" << type
				<< ":" << subtype << ":" << size);
		MemoryInfo memoryInfo;
		memoryInfo.memoryPtr = (char*) malloc(size + 1);
		memoryInfo.memoryPtr[size] = NULL;
		memoryInfo.size = size;
		memoryInfo.type = (char*) malloc(MAX_TYPE_SIZE);
		memset(memoryInfo.type, '\0', MAX_TYPE_SIZE);
		strncpy(memoryInfo.type, type, MAX_TYPE_SIZE);
		memoryInfo.subtype = (char*) malloc(MAX_SUBTYPE_SIZE);
		memset(memoryInfo.subtype, '\0', MAX_SUBTYPE_SIZE);
		strncpy(memoryInfo.subtype, subtype, MAX_SUBTYPE_SIZE);

		LOG4CXX_DEBUG(
				logger,
				(char*) "adding MemoryInfo: %p with type: %s with subtype: %s to vector"
						<< (char*) memoryInfo.memoryPtr << ":"
						<< (char*) memoryInfo.type << ":"
						<< (char*) memoryInfo.subtype);
		memoryInfoVector.push_back(memoryInfo);
		LOG4CXX_DEBUG(logger, (char*) "added MemoryInfo to vector");
		toReturn = (char*) memoryInfo.memoryPtr;
	}
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "tpalloc unlocked");
	return toReturn;
}

char* AtmiBrokerMem::tprealloc(char * addr, long size) {
	char* toReturn = NULL;
	LOG4CXX_TRACE(logger, (char*) "tprealloc locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "tprealloc locked");
	if (!addr) {
		LOG4CXX_ERROR(logger, (char*) "tprealloc - no buffer");
		setSpecific(TPE_KEY, bTPEINVAL);
	} else if (size < 0) {
		LOG4CXX_ERROR(logger, (char*) "tprealloc - negative size");
		setSpecific(TPE_KEY, bTPEINVAL);
	} else {
		LOG4CXX_DEBUG(logger, (char*) "tprealloc - addr: %p size: %d" << addr
				<< ":" << size);
		for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it
				!= memoryInfoVector.end(); it++) {
			LOG4CXX_TRACE(logger, (char*) "next memoryInfo id is: %p"
					<< (char*) (*it).memoryPtr);
			if ((*it).memoryPtr == addr) {
				LOG4CXX_DEBUG(logger, (char*) "found matching memory %p"
						<< (*it).memoryPtr);
				LOG4CXX_DEBUG(logger, (char*) "updating memory ptr %p"
						<< (*it).memoryPtr);

				if (strncmp((*it).type, "X_COMMON", 8) == 0 || strncmp(
						(*it).type, "X_C_TYPE", 8) == 0) {
					if (size < 1024)
						size = 1024;
				}

				char* memPtr = (char*) realloc((void*) addr, size + 1);
				(*it).memoryPtr = memPtr;
				(*it).memoryPtr[size] = NULL;
				(*it).size = size;
				LOG4CXX_DEBUG(logger, (char*) "updated memory ptr %p"
						<< (*it).memoryPtr);
				toReturn = memPtr;
				break;
			}
		}

		if (toReturn == NULL) {
			LOG4CXX_DEBUG(logger,
					(char*) "tprealloc - not found addr: %p size: %d" << addr
							<< ":" << size);
			setSpecific(TPE_KEY, bTPEINVAL);
			LOG4CXX_DEBUG(logger,
					(char*) "tprealloc - not found addr: %p  failure advised"
							<< addr);
		}
	}
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "tprealloc unlocked");
	return toReturn;
}

void AtmiBrokerMem::tpfree(char* ptr) {
	bool found = false;
	LOG4CXX_TRACE(logger, (char*) "tpfree locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "tpfree locked");
	if (ptr && ptr != NULL) {
		LOG4CXX_DEBUG(logger, (char*) "tpfree - ptr: %p" << ptr);
		for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it
				!= memoryInfoVector.end(); it++) {
//			LOG4CXX_TRACE(logger, (char*) "next memoryInfo id is: %p"
//					<< (char*) (*it).memoryPtr);
			if ((*it).memoryPtr == NULL) {
				LOG4CXX_ERROR(logger, (char*) "found a null in the vector");
				break;
			} else if ((*it).memoryPtr == ptr) {
				MemoryInfo memoryInfo = (*it);
				LOG4CXX_DEBUG(logger, (char*) "freeing memoryPtr: %p"
						<< (char*) memoryInfo.memoryPtr);
				free(memoryInfo.memoryPtr);
				LOG4CXX_DEBUG(logger, (char*) "freeing type: %p"
						<< (char*) memoryInfo.type);
				free(memoryInfo.type);
				LOG4CXX_DEBUG(logger, (char*) "freeing subtype: %p"
						<< (char*) memoryInfo.subtype);
				free(memoryInfo.subtype);
				LOG4CXX_DEBUG(logger, (char*) "freed memory");

				LOG4CXX_DEBUG(logger, (char*) "removing %p from vector");
				memoryInfoVector.erase(it);
				LOG4CXX_DEBUG(logger, (char*) "removed from vector ");

				found = true;
				break;
			}
		}
	}
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "tpfree unlocked");
	return;
}

long AtmiBrokerMem::tptypes(char* ptr, char* type, char* subtype) {
	LOG4CXX_TRACE(logger, (char*) "tptypes locking");
	lock->lock();
	LOG4CXX_TRACE(logger, (char*) "tptypes locked");
	long toReturn = -1;
	if (ptr && ptr != NULL) {
		LOG4CXX_DEBUG(logger, (char*) "tptypes - ptr: %p" << ptr);
		for (std::vector<MemoryInfo>::iterator it = memoryInfoVector.begin(); it
				!= memoryInfoVector.end(); it++) {
			LOG4CXX_TRACE(logger, (char*) "next memoryInfo id is: %p"
					<< (char*) (*it).memoryPtr);
			if ((*it).memoryPtr == ptr) {
				MemoryInfo memoryInfo = (*it);
				LOG4CXX_DEBUG(logger, (char*) "found matching memory %p"
						<< memoryInfo.memoryPtr);
				LOG4CXX_DEBUG(logger, (char*) "type is %s"
						<< (char*) memoryInfo.type);
				LOG4CXX_DEBUG(logger, (char*) "subtype is %s"
						<< (char*) memoryInfo.subtype);

				if (type) {
					strncpy(type, memoryInfo.type, MAX_TYPE_SIZE);
				}
				if (subtype) {
					strncpy(subtype, memoryInfo.subtype, MAX_SUBTYPE_SIZE);
				}
				toReturn = memoryInfo.size;
				break;
			}
		}
	}
	if (toReturn == -1) {
		// WAS NOT FOUND
		setSpecific(TPE_KEY, bTPEINVAL);
	}
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "tptypes unlocked");
	return toReturn;
}
