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
		
		LOG4CXX_TRACE(logger, (char*) "allocated: " << *length);
		if (*length != 0) {
			memcpy(data_togo, membuffer, *length);
			LOG4CXX_TRACE(logger, (char*) "copied: idata into: data_togo");
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
	}
	return data_tostay;
}
