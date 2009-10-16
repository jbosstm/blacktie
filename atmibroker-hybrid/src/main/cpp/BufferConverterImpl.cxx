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

char* BufferConverterImpl::convertToWireFormat(char* bufferSubtype,
		char* memoryFormatBuffer, int* wireFormatBufferLength) {
	LOG4CXX_DEBUG(logger, (char*) "convertToWireFormat");

	Buffer* buffer = buffers[bufferSubtype];
	char* data_togo = new char[buffer->wireSize + 1];
	LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->wireSize + 1);
	data_togo[buffer->wireSize] = NULL;

	// Copy the attributes in
	int copiedAmount = 0;
	// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
	Attributes::iterator i;
	for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
		Attribute* attribute = i->second;
		memcpy(&data_togo[attribute->wirePosition],
				&memoryFormatBuffer[attribute->memPosition],
				attribute->instanceSize);
		copiedAmount = copiedAmount + attribute->instanceSize;
		LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
				<< attribute->instanceSize);
	}

	if (copiedAmount != buffer->wireSize) {
		LOG4CXX_ERROR(logger, (char*) "DID NOT FILL THE BUFFER Amount: "
				<< copiedAmount << " Expected: " << buffer->wireSize);
	}
	*wireFormatBufferLength = copiedAmount;
	return data_togo;
}

char* BufferConverterImpl::convertToMemoryFormat(char* bufferSubtype,
		char* memoryFormatBuffer, int* memoryFormatBufferLength) {
	LOG4CXX_DEBUG(logger, (char*) "convertToMemoryFormat");

	Buffer* buffer = buffers[bufferSubtype];
	char* data_tostay = new char[buffer->memSize];
	LOG4CXX_TRACE(logger, (char*) "allocated: " << buffer->memSize);

	// Copy the attributes in
	int copiedAmount = 0;
	// TODO ASSUMES ATMIBROKERMEM HAS INITED THE MEMORY WITH DETAILS
	Attributes::iterator i;
	for (i = buffer->attributes.begin(); i != buffer->attributes.end(); ++i) {
		Attribute* attribute = i->second;
		memcpy(&data_tostay[attribute->memPosition],
				&memoryFormatBuffer[attribute->wirePosition],
				attribute->instanceSize);
		copiedAmount = copiedAmount + attribute->instanceSize;
		LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo: "
				<< attribute->instanceSize);
	}

	if ((copiedAmount + buffer->lastPad) != buffer->memSize) {
		LOG4CXX_ERROR(logger, (char*) "DID NOT FILL THE BUFFER Amount: "
				<< (copiedAmount + buffer->lastPad) << " Expected: "
				<< buffer->wireSize);
	}
	*memoryFormatBufferLength = (copiedAmount + buffer->lastPad);
	return data_tostay;
}
