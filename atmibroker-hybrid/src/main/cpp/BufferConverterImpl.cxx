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

#include "BufferConverterImpl.h"

#include <exception>
#include "malloc.h"

#include "AtmiBrokerEnvXml.h"

log4cxx::LoggerPtr BufferConverterImpl::logger(log4cxx::Logger::getLogger(
		"BufferConverterImpl"));

char* BufferConverterImpl::convertToWireFormat(char* bufferType,
		char* bufferSubtype, char* memoryFormatBuffer,
		long* wireFormatBufferLength) {
	LOG4CXX_DEBUG(logger, (char*) "convertToWireFormat");
	char* data_togo = NULL;

	if (strlen(bufferType) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Sending NULL buffer");
		*wireFormatBufferLength = 1;
		data_togo = new char[*wireFormatBufferLength];
		data_togo[0] = NULL;
	} else if (strncmp(bufferType, "BT_NBF", 6) == 0) {
		data_togo = new char[*wireFormatBufferLength];
		
		LOG4CXX_TRACE(logger, (char*) "allocated: " << *wireFormatBufferLength);
		if (*wireFormatBufferLength != 0) {
			memcpy(data_togo, memoryFormatBuffer, *wireFormatBufferLength);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
		}
	} else if (strncmp(bufferType, "X_OCTET", 8) == 0) {
		data_togo = new char[*wireFormatBufferLength];

		LOG4CXX_TRACE(logger, (char*) "allocated: " << *wireFormatBufferLength);
		if (*wireFormatBufferLength != 0) {
			memcpy(data_togo, memoryFormatBuffer, *wireFormatBufferLength);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
		}
	} else {
		Buffer* buffer = buffers[bufferSubtype];
		data_togo = new char[buffer->wireSize];
		memset(data_togo, '\0', buffer->wireSize);
		LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->wireSize);

		// Copy the attributes in
		int copiedAmount = 0;
		// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
		Attributes::iterator i;
		for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
			Attribute* attribute = i->second;
			memcpy(&data_togo[attribute->wirePosition],
					&memoryFormatBuffer[attribute->memPosition],
					attribute->memSize);
			copiedAmount = copiedAmount + attribute->memSize;
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
					<< attribute->memSize);
		}

		if (copiedAmount != buffer->wireSize) {
			LOG4CXX_TRACE(logger, (char*) "DID NOT FILL THE BUFFER Amount: "
					<< copiedAmount << " Expected: " << buffer->wireSize);
		}
		*wireFormatBufferLength = buffer->wireSize;
	}
	return data_togo;
}

char* BufferConverterImpl::convertToMemoryFormat(char* bufferType,
		char* bufferSubtype, char* memoryFormatBuffer,
		long* memoryFormatBufferLength) {
	LOG4CXX_DEBUG(logger, (char*) "convertToMemoryFormat");
	char* data_tostay = NULL;

	if (strlen(bufferType) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received NULL buffer");
		*memoryFormatBufferLength = 0;
	} else if (strncmp(bufferType, "BT_NBF", 6) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received an BT_NBF buffer");
		*memoryFormatBufferLength = *memoryFormatBufferLength;
		LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
		data_tostay = (char*) malloc(*memoryFormatBufferLength);
		LOG4CXX_TRACE(logger, (char*) "Allocated");
		if (*memoryFormatBufferLength > 0) {
			memcpy(data_tostay, memoryFormatBuffer, *memoryFormatBufferLength);
			LOG4CXX_TRACE(logger, (char*) "Copied");
		}
	} else if (strncmp(bufferType, "X_OCTET", 8) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received an X_OCTET buffer");
		*memoryFormatBufferLength = *memoryFormatBufferLength;
		LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
		data_tostay = (char*) malloc(*memoryFormatBufferLength);
		LOG4CXX_TRACE(logger, (char*) "Allocated");
		if (*memoryFormatBufferLength > 0) {
			memcpy(data_tostay, memoryFormatBuffer, *memoryFormatBufferLength);
			LOG4CXX_TRACE(logger, (char*) "Copied");
		}
	} else {
		LOG4CXX_TRACE(logger, (char*) "Received a non X_OCTET buffer: "
				<< bufferSubtype);
		Buffer* buffer = buffers[bufferSubtype];
		if (buffer == NULL) {
			LOG4CXX_FATAL(
					logger,
					(char*) "Unknown buffer type: "
							<< bufferSubtype);
		}


		if (*memoryFormatBufferLength != buffer->wireSize) {
			LOG4CXX_ERROR(
					logger,
					(char*) "DID NOT Receive the expected amount of wire data: "
							<< *memoryFormatBufferLength << " Expected: "
							<< buffer->wireSize);
		}
		data_tostay = (char*) malloc(buffer->memSize);

		memset(data_tostay, '\0', buffer->memSize);
		LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->memSize);

		// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
		Attributes::iterator i;
		for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
			Attribute* attribute = i->second;
			memcpy(&data_tostay[attribute->memPosition],
					&memoryFormatBuffer[attribute->wirePosition],
					attribute->wireSize);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
					<< attribute->wireSize);
		}
		*memoryFormatBufferLength = buffer->memSize;
	}
	return data_tostay;
}
