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

#include "DefaultCodecImpl.h"

#include <exception>
#include "malloc.h"
#include "ace/os_include/netinet/os_in.h"
#include "ace/Basic_Types.h"
#include "AtmiBrokerEnvXml.h"

log4cxx::LoggerPtr DefaultCodecImpl::logger(log4cxx::Logger::getLogger(
		"DefaultCodecImpl"));

ACE_UINT64 htonll(ACE_UINT64 value) {
	static const int num = 42;

	if (*reinterpret_cast<const char*>(&num) == num) {
		const ACE_UINT32 high_part = htonl(static_cast<ACE_UINT32>(value >> 32));
		const ACE_UINT32 low_part  = htonl(static_cast<ACE_UINT32>(value & 0xFFFFFFFFLL));

		return (static_cast<ACE_UINT64>(low_part) << 32) | high_part;
	} else {
		return value;
	}
}

ACE_UINT64 ntohll(ACE_UINT64 value) {
	static const int num = 42;

	if (*reinterpret_cast<const char*>(&num) == num) {
		const ACE_UINT32 high_part = ntohl(static_cast<ACE_UINT32>(value >> 32));
		const ACE_UINT32 low_part  = ntohl(static_cast<ACE_UINT32>(value & 0xFFFFFFFFLL));

		return (static_cast<ACE_UINT64>(low_part) << 32) | high_part;
	} else {
		return value;
	}
}

char* DefaultCodecImpl::encode(char* type,
		char* subtype, char* membuffer,
		long* length) {
	LOG4CXX_DEBUG(logger, (char*) "convertToWireFormat");
	char* data_togo = NULL;

	if (strlen(type) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Sending NULL buffer");
		*length = 1;
		data_togo = new char[*length];
		data_togo[0] = (char) NULL;
	} else if (strncmp(type, "BT_NBF", 6) == 0) {
		data_togo = new char[*length];

		LOG4CXX_TRACE(logger, (char*) "allocated: " << *length);
		if (*length != 0) {
			memcpy(data_togo, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
		}
	} else if (strncmp(type, "X_OCTET", 8) == 0) {
		data_togo = new char[*length];

		LOG4CXX_TRACE(logger, (char*) "allocated: " << *length);
		if (*length != 0) {
			memcpy(data_togo, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
		}
	} else {
		Buffer* buffer = buffers[subtype];
		data_togo = new char[buffer->wireSize];
		memset(data_togo, '\0', buffer->wireSize);
		LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->wireSize);

		// Copy the attributes in
		int copiedAmount = 0;
		// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
		Attributes::iterator i;
		for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
			Attribute* attribute = i->second;
			char* buf;
			ACE_UINT16 svalue;
			ACE_UINT32 lvalue;
			ACE_UINT64 llvalue;

			if (strcmp(attribute->type, "short") == 0) {
				svalue = *(ACE_UINT16*)(&membuffer[attribute->memPosition]);
				svalue = htons(svalue);
				LOG4CXX_DEBUG(logger, (char*) "htons short value:" << svalue);
				buf = (char*)&svalue;
			} else if((strcmp(attribute->type, "long") == 0 && sizeof(long) == 4) || strcmp(attribute->type, "int") == 0) {
				lvalue = *(ACE_UINT32*)(&membuffer[attribute->memPosition]);
				lvalue = htonl(lvalue);
				LOG4CXX_DEBUG(logger, (char*) "htonl long value:" << lvalue);
				buf = (char*)&lvalue;
			} else if(strcmp(attribute->type, "long") == 0 && sizeof(long) == 8) {
				llvalue = *(ACE_UINT64*)(&membuffer[attribute->memPosition]);
				llvalue = htonll(llvalue);
				LOG4CXX_DEBUG(logger, (char*) "htonll long64 value:" << llvalue);
				buf = (char*)&llvalue;
			} else {
				buf = &membuffer[attribute->memPosition];
			}
			memcpy(&data_togo[attribute->wirePosition], buf, attribute->memSize);
			copiedAmount = copiedAmount + attribute->memSize;
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
					<< attribute->memSize);
		}

		if (copiedAmount != buffer->wireSize) {
			LOG4CXX_TRACE(logger, (char*) "DID NOT FILL THE BUFFER Amount: "
					<< copiedAmount << " Expected: " << buffer->wireSize);
		}
		*length = buffer->wireSize;
	}
	return data_togo;
}

char* DefaultCodecImpl::decode(char* type,
		char* subtype, char* membuffer,
		long* length) {
	LOG4CXX_DEBUG(logger, (char*) "convertToMemoryFormat");
	char* data_tostay = NULL;

	if (strlen(type) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received NULL buffer");
		*length = 0;
	} else if (strncmp(type, "BT_NBF", 6) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received an BT_NBF buffer");
		*length = *length;
		LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
		data_tostay = (char*) malloc(*length);
		LOG4CXX_TRACE(logger, (char*) "Allocated");
		if (*length > 0) {
			memcpy(data_tostay, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "Copied");
		}
	} else if (strncmp(type, "X_OCTET", 8) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received an X_OCTET buffer");
		*length = *length;
		LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
		data_tostay = (char*) malloc(*length);
		LOG4CXX_TRACE(logger, (char*) "Allocated");
		if (*length > 0) {
			memcpy(data_tostay, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "Copied");
		}
	} else {
		LOG4CXX_TRACE(logger, (char*) "Received a non X_OCTET buffer: "
				<< subtype);
		Buffer* buffer = buffers[subtype];
		if (buffer == NULL) {
			LOG4CXX_FATAL(
					logger,
					(char*) "Unknown buffer type: "
					<< subtype);
		}


		if (*length != buffer->wireSize) {
			LOG4CXX_ERROR(
					logger,
					(char*) "DID NOT Receive the expected amount of wire data: "
					<< *length << " Expected: "
					<< buffer->wireSize);
		}
		data_tostay = (char*) malloc(buffer->memSize);

		memset(data_tostay, '\0', buffer->memSize);
		LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->memSize);

		// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
		Attributes::iterator i;
		for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
			Attribute* attribute = i->second;
			if(strcmp(attribute->type, "short") == 0) {
				ACE_UINT16 value = *((ACE_UINT16*)&membuffer[attribute->wirePosition]);
				value = ntohs(value);
				LOG4CXX_DEBUG(logger, (char*) "ntohs short value:" << value);
				memcpy(&membuffer[attribute->wirePosition], &value, attribute->wireSize);
			} else if((strcmp(attribute->type, "long") == 0 && sizeof(long) == 4) || strcmp(attribute->type, "int") == 0) {
				ACE_UINT32 value = *((ACE_UINT32*)&membuffer[attribute->wirePosition]);
				value = ntohl(value);
				LOG4CXX_DEBUG(logger, (char*) "ntohl long value:" << value);
				memcpy(&membuffer[attribute->wirePosition], &value, attribute->wireSize);
			} else if(strcmp(attribute->type, "long") == 0 && sizeof(long) == 8) {
				ACE_UINT64 value = *((ACE_UINT64*)&membuffer[attribute->wirePosition]);
				value = ntohll(value);
				LOG4CXX_DEBUG(logger, (char*) "ntohll long64 value:" << value);
				memcpy(&membuffer[attribute->wirePosition], &value, attribute->wireSize);
			}
			memcpy(&data_tostay[attribute->memPosition],
					&membuffer[attribute->wirePosition],
					attribute->wireSize);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
					<< attribute->wireSize);
		}
		*length = buffer->memSize;
	}
	return data_tostay;
}
