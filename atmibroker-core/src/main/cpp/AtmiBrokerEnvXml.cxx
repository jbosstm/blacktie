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

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "expat.h"

#include "AtmiBrokerEnvXml.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerEnvXml(Logger::getLogger("AtmiBrokerEnvXml"));

char* notifyServiceId;
char* namingServiceId;
char* loggingServiceId;
char domain[30];
char* queue_name;
char* xaResourceMgrId;
char* transFactoryId;
char* xaResourceName;
char* xaOpenString;
char* xaCloseString;
bool xaThreadModel;
bool xaAutomaticAssociation;
bool xaDynamicRegistrationOptimization;

static char last_element[50];
static char last_value[1024];

static char element[50];
static char value[1024];

static int depth = 0;

static int envVariableCount = 0;

static bool processingEnvironment = false;
static bool processingServer = false;
static bool processingQSpaceName = false;
static bool processingNamingServiceId = false;
static bool processingNotifyServiceId = false;
static bool processingLoggingServiceId = false;
static bool processingTransFactoryId = false;
static bool processingXaResourceMgrId = false;
static bool processingXaResourceName = false;
static bool processingXaOpenString = false;
static bool processingXaCloseString = false;
static bool processingXaThreadModel = false;
static bool processingXaAutomaticAssociation = false;
static bool processingXaDynamicRegistrationOptimization = false;
static bool processingDomain = false;
static bool processingEnvVariables = false;
static bool processingEnvVariable = false;
static bool processingEnvName = false;
static bool processingEnvValue = false;

AtmiBrokerEnvXml::AtmiBrokerEnvXml() {
	depth = 0;
	envVariableCount = 0;

	processingEnvironment = false;
	processingServer = false;
	processingQSpaceName = false;
	processingNamingServiceId = false;
	processingNotifyServiceId = false;
	processingLoggingServiceId = false;
	processingTransFactoryId = false;
	processingXaResourceMgrId = false;
	processingXaResourceName = false;
	processingXaOpenString = false;
	processingXaCloseString = false;
	processingXaThreadModel = false;
	processingXaAutomaticAssociation = false;
	processingXaDynamicRegistrationOptimization = false;
	processingDomain = false;
	processingEnvVariables = false;
	processingEnvVariable = false;
	processingEnvName = false;
	processingEnvValue = false;
}

AtmiBrokerEnvXml::~AtmiBrokerEnvXml() {
}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	if (strcmp(name, "ENVIRONMENT xmnls") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "new server ");
		processingEnvironment = true;

	} else if (strcmp(name, "SERVER") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Server for environment ");
		processingServer = true;
	} else if (strcmp(name, "DOMAIN") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Domain for environment ");
		processingDomain = true;
	} else if (strcmp(name, "QSPACE_NAME") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing QSpaceName for environment ");
		processingQSpaceName = true;
	} else if (strcmp(name, "NAMING_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing NamingServiceId for environment ");
		processingNamingServiceId = true;
	} else if (strcmp(name, "NOTIFY_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing NotifyServiceId for environment ");
		processingNotifyServiceId = true;
	} else if (strcmp(name, "LOGGING_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing LoggingServiceId for environment ");
		processingLoggingServiceId = true;
	} else if (strcmp(name, "TRANS_FACTORY_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing TransFactoryId for environment ");
		processingTransFactoryId = true;
	} else if (strcmp(name, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaResourceMgrId for environment ");
		processingXaResourceMgrId = true;
	} else if (strcmp(name, "XA_RESOURCE_NAME") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaResourceName for environment ");
		processingXaResourceName = true;
	} else if (strcmp(name, "XA_OPEN_STRING") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaOpenString for environment ");
		processingXaOpenString = true;
	} else if (strcmp(name, "XA_CLOSE_STRING") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaCloseString for environment ");
		processingXaCloseString = true;
	} else if (strcmp(name, "XA_THREAD_MODEL") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaThreadModel for environment ");
		processingXaThreadModel = true;
	} else if (strcmp(name, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaAutomaticAssociation for environment ");
		processingXaAutomaticAssociation = true;
	} else if (strcmp(name, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing XaDynamicRegistrationOptimization for environment ");
		processingXaDynamicRegistrationOptimization = true;
	} else if (strcmp(name, "ENV_VARIABLES") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Env Variables for environment ");
		processingEnvVariables = true;
	} else if (strcmp(name, "ENV_VARIABLE") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Env Variable for environment ");
		processingEnvVariable = true;
		envVariableCount++;
		envVar_t envVar;
		(*aEnvironmentStructPtr).push_back(envVar);
	} else if (strcmp(last_element, "NAME") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Env Name for environment ");
		processingEnvName = true;
	} else if (strcmp(last_element, "VALUE") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing Env Value for environment ");
		processingEnvValue = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL
endElement(void *userData, const char *name) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	bool storedElement = false;
	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "DOMAIN") == 0) {
		storedElement = true;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing Domain %s" << last_value);
		processingDomain = false;
		strcpy(domain, last_value);
	} else if (strcmp(last_element, "QSPACE_NAME") == 0) {
		storedElement = true;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing QSpaceName %s" << last_value);
		processingQSpaceName = false;
		queue_name = strdup(last_value);
	} else if (strcmp(last_element, "NAMING_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing NamingServiceId %s" << last_value);
		processingNamingServiceId = false;
		namingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "NOTIFY_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing NotifyServiceId %s" << last_value);
		processingNotifyServiceId = false;
		notifyServiceId = strdup(last_value);
	} else if (strcmp(last_element, "LOGGING_SERVICE_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing LoggingServiceId %s" << last_value);
		processingLoggingServiceId = false;
		loggingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "TRANS_FACTORY_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing TransFactoryId %s" << last_value);
		processingTransFactoryId = false;
		transFactoryId = strdup(last_value);
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaResourceMgrId %s" << last_value);
		processingXaResourceMgrId = false;
		xaResourceMgrId = strdup(last_value);
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaResourceName %s" << last_value);
		processingXaResourceName = false;
		xaResourceName = strdup(last_value);
	} else if (strcmp(last_element, "XA_OPEN_STRING") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaOpenString %s" << last_value);
		processingXaOpenString = false;
		xaOpenString = strdup(last_value);
	} else if (strcmp(last_element, "XA_CLOSE_STRING") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaCloseString %s" << last_value);
		processingXaCloseString = false;
		xaCloseString = strdup(last_value);
	} else if (strcmp(last_element, "XA_THREAD_MODEL") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaThreadModel %s" << last_value);
		processingXaThreadModel = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaThreadModel = true;
		else
			xaThreadModel = false;
	} else if (strcmp(last_element, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaAutomaticAssociation %s" << last_value);
		processingXaAutomaticAssociation = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaAutomaticAssociation = true;
		else
			xaAutomaticAssociation = false;
	} else if (strcmp(last_element, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing XaDynamicRegistrationOptimization %s" << last_value);
		processingXaDynamicRegistrationOptimization = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaDynamicRegistrationOptimization = true;
		else
			xaDynamicRegistrationOptimization = false;
	} else if (strcmp(last_element, "ENV_VARIABLES") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "stored EnvVariables ");
		processingEnvVariables = false;
	} else if (strcmp(last_element, "ENV_VARIABLE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "stored EnvVariable at index %d" << index);
		processingEnvVariable = false;
	} else if (strcmp(last_element, "NAME") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing EnvName '%s' at index %d" << last_value << index);
		processingEnvName = false;
		(*aEnvironmentStructPtr)[index].name = strdup(last_value);
	} else if (strcmp(last_element, "VALUE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing Env Value '%s' at index %d" << last_value << index);
		processingEnvValue = false;
		(*aEnvironmentStructPtr)[index].value = strdup(last_value);
	}
	depth -= 1;
}

static void XMLCALL
characterData(void *userData, const char *cdata, int len) {
	//AtmiBroker::EnvVariableInfoSeq* aEnvironmentStructPtr = (AtmiBroker::EnvVariableInfoSeq*)userData;

	int i = 0;
	int j = 0;
	int priorLength = strlen(value);

	i = priorLength;
	for (; i < len + priorLength; i++, j++) {
		value[i] = cdata[j];
	}
	value[i] = '\0';
	LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "value is '%s'" << value);
}

bool AtmiBrokerEnvXml::parseXmlDescriptor(std::vector<envVar_t>* aEnvironmentStructPtr, const char * aDescriptorFileName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "in parseXmlDescriptor() %s" << aDescriptorFileName);

	bool toReturn = true;

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	if (!aDescriptorFile) {
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*)"loadfile: fopen failed on %s" << aDescriptorFileName);
		return false;
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "read file %p" << aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*)"loadfile: fstat failed on %s" << aDescriptorFileName);
	}
	if (s.st_size == 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*)"loadfile: file %s is empty" << aDescriptorFileName);
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*)"loadfile: file %s is %d long" << aDescriptorFileName << s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*)"loadfile: Could not allocate enough memory to load file %s" << aDescriptorFileName);
	}
	for (unsigned int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*)"loadfile: Allocated enough memory to load file %d" << s.st_size);

	namingServiceId = strdup("");
	notifyServiceId = strdup("");;
	loggingServiceId = strdup("");

	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	strcpy(element, "");
	strcpy(value, "");
	XML_SetUserData(parser, aEnvironmentStructPtr);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	do {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "reading file");
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "length is '%d'" << len);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) "%d at line %d" << XML_ErrorString(XML_GetErrorCode(parser)) << XML_GetCurrentLineNumber(parser));
			toReturn = false;
			break;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "leaving parseXmlDescriptor() %s" << aDescriptorFileName);
	return toReturn;
}

