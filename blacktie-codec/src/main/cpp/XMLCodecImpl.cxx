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
#include "XMLCodecImpl.h"
#include "XMLCodecParser.h"
#include "Base64.h"
#include "AtmiBrokerEnv.h"

log4cxx::LoggerPtr XMLCodecImpl::logger(log4cxx::Logger::getLogger(
		"XMLCodecImpl"));

char* XMLCodecImpl::encode(char* type, char* subtype, 
		char* membuffer, long* length) {
	char* data_togo = NULL;

	LOG4CXX_DEBUG(logger, (char*) "XMLCodecImpl::encode");
	if (strlen(type) == 0) {
		LOG4CXX_TRACE(logger, (char*) "Sending NULL buffer");
		*length = 1;
		data_togo = new char[*length];
		data_togo[0] = (char) NULL;
	} else if(strcmp(type, "BT_NBF") == 0) {
		data_togo = new char[*length];
		
		LOG4CXX_TRACE(logger, (char*) "BT_NBF allocated: " << *length);
		if (*length != 0) {
			memcpy(data_togo, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
		}
	} else if(strcmp(type, "X_OCTET") == 0) {
		char* content;
		content = base64_encode(membuffer, length);
		long len = *length;
		if(content != NULL) {
			*length += 256;
			data_togo = new char[*length];
			memset(data_togo, '\0', *length);

			LOG4CXX_TRACE(logger, (char*) "X_OCTET allocated: " << *length);
			strcpy(data_togo, "<?xml version='1.0'?>");
			strcat(data_togo, "<bt:X_OCTET");
			strcat(data_togo, " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"");
			strcat(data_togo, " xmlns:bt=\"http://www.jboss.org/blacktie\"");
			strcat(data_togo, " xsi:schemaLocation=\"http://www.jboss.org/blacktie buffers/X_OCTET.xsd\">");
			strncat(data_togo, content, len-1);
			strcat(data_togo,"</bt:X_OCTET>");
			delete content;
		} else {
			LOG4CXX_ERROR(logger, (char*) "X_OCTET: base64 encode failed");
		}
	}
	return data_togo;
}

char* XMLCodecImpl::decode(char* type, char* subtype, 
		char* membuffer, long* length) {
	char* data_tostay = NULL;

	LOG4CXX_DEBUG(logger, (char*) "XMLCodecImpl::decode");
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
	} else if (strcmp(type, "X_OCTET") == 0) {
		LOG4CXX_TRACE(logger, (char*) "Received an X_OCTET buffer");
		XMLCodecParser parser;
		XMLCodecHandlers handler(type, subtype);

		parser.parse(membuffer, *length, &handler);
		char* content = handler.getBuffer();
		*length = handler.getLength();
		LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
		data_tostay = (char*) malloc (*length);
		LOG4CXX_TRACE(logger, (char*) "Allocated");
		if (*length > 0) {
			memcpy(data_tostay, content, *length);
			LOG4CXX_TRACE(logger, (char*) "Copied");
			delete content;
			LOG4CXX_TRACE(logger, (char*) "Delete content");
		}
	}
	return data_tostay;
}
