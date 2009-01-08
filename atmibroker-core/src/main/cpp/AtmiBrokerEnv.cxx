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

/*
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#endif
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerEnv(Logger::getLogger("AtmiBrokerEnv"));

int AtmiBrokerEnv::ENV_VARIABLE_SIZE = 30;
char *AtmiBrokerEnv::ENVIRONMENT_FILE = (char*) "Environment.xml";
AtmiBrokerEnv *AtmiBrokerEnv::ptrAtmiBrokerEnv = NULL;

AtmiBrokerEnv *
AtmiBrokerEnv::get_instance() {
	if (ptrAtmiBrokerEnv == NULL)
		ptrAtmiBrokerEnv = new AtmiBrokerEnv();
	return ptrAtmiBrokerEnv;
}

void AtmiBrokerEnv::discard_instance() {
	if (ptrAtmiBrokerEnv != NULL) {
		delete ptrAtmiBrokerEnv;
		ptrAtmiBrokerEnv = NULL;
	}
}

AtmiBrokerEnv::AtmiBrokerEnv() {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "constructor");
	readEnvironment = false;
	// THIS WILL LOAD THE ENV FILE IF NOT LOADED
	readenv(NULL, NULL);
}

AtmiBrokerEnv::~AtmiBrokerEnv() {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "destructor");
	for (unsigned int i = 0; i < envVariableInfoSeq.length(); i++) {
		//free((char*)envVariableInfoSeq[i].name);
		//free((char*)envVariableInfoSeq[i].value);
	}
	//envVariableInfoSeq.length(0);
	envVariableInfoSeq = NULL;
	readEnvironment = false;
}

char*
AtmiBrokerEnv::getenv(char* anEnvName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "getenv %s" << anEnvName);

	char *envValue = ::getenv(anEnvName);
	if (envValue != NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "getenv env is %s"<< envValue);
		return strdup(envValue);
	}

	for (unsigned int i = 0; i < envVariableInfoSeq.length(); i++) {
		if (strcmp(anEnvName, envVariableInfoSeq[i].name) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "getenv found env name '%s'" << (const char*)envVariableInfoSeq[i].value);
			const char * aValue = (const char *) envVariableInfoSeq[i].value;
			return (char *) aValue;
		}
	}
	if (envValue != NULL) {
		return strdup(envValue);
	}
	return NULL;
}

int AtmiBrokerEnv::putenv(char* anEnvNameValue) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "putenv %s" << anEnvNameValue);

	::putenv(anEnvNameValue);

	int size = strlen(anEnvNameValue);

	char * name = (char*) malloc(sizeof(char) * size);
	memset(name, '\0', size);
	char * value = (char*) malloc(sizeof(char) * size);
	memset(value, '\0', size);

	int i = 0;
	for (; i < size; i++) {
		if (anEnvNameValue[i] == '=')
			break;
		else
			name[i] = anEnvNameValue[i];
	}
	int j = 0;
	i++;
	for (; i < size; i++, j++) {
		value[j] = anEnvNameValue[i];
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "putenv name '%s' value '%s'" << name << value);

	int numEnvs = envVariableInfoSeq.length();
	envVariableInfoSeq.length(numEnvs + 1);
	envVariableInfoSeq[numEnvs].name = strdup(name);
	envVariableInfoSeq[numEnvs].value = strdup(value);
	free(name);
	free(value);

	AtmiBrokerEnvXml aAtmiBrokerEnvXml;
	aAtmiBrokerEnvXml.writeXmlDescriptor(&envVariableInfoSeq, ENVIRONMENT_FILE);

	return 1;
}

int AtmiBrokerEnv::readenv(char* aEnvFileName, char* label) {
	if (!readEnvironment) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "readenv ignores label variable");
		AtmiBrokerEnvXml aAtmiBrokerEnvXml;

		if (aEnvFileName != NULL) {
			LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "readenv	file now: %s " << aEnvFileName);
			ENVIRONMENT_FILE = aEnvFileName;
		}

		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "readenv file: %s " << ENVIRONMENT_FILE);
		if (aAtmiBrokerEnvXml.parseXmlDescriptor(&envVariableInfoSeq, ENVIRONMENT_FILE)) {
			readEnvironment = true;
		} else {
			return -1;
		}
	}
	return 1;
}

AtmiBroker::EnvVariableInfoSeq &
AtmiBrokerEnv::getEnvVariableInfoSeq() {
	return envVariableInfoSeq;
}

