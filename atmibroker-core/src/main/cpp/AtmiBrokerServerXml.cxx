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

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "expat.h"

#include "AtmiBrokerServerXml.h"
#include "XsdValidator.h"
#include "userlog.h"
#include "log4cxx/logger.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"

log4cxx::LoggerPtr loggerAtmiBrokerServerXml(log4cxx::Logger::getLogger(
		"AtmiBrokerServerXml"));

char server[30];

int serverMaxChannels = 0;
int serverMaxSuppliers = 0;
int serverMaxConsumers = 0;

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static int serviceNameCount = 0;

static bool processingServer = false;
static bool processingName = false;
static bool processingMaxSuppliers = false;
static bool processingMaxConsumers = false;
static bool processingOrbType = false;
static bool processingMaxChannels = false;
static bool processingServiceNames = false;
static bool processingServiceName = false;

AtmiBrokerServerXml::AtmiBrokerServerXml() {
	depth = 0;
	serviceNameCount = 0;
}

AtmiBrokerServerXml::~AtmiBrokerServerXml() {
}

static void XMLCALL startElement(void *userData, const char *name, const char **atts) {
	if (strcmp(name, "SERVER xmnls") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "new server ");
		processingServer = true;

	} else if (strcmp(name, "NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing NAME for server ");
		processingName = true;
	} else if (strcmp(name, "MAX_CHANNELS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Channels for server ");
		processingMaxChannels = true;
	} else if (strcmp(name, "MAX_SUPPLIERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Suppliers for server ");
		processingMaxSuppliers = true;
	} else if (strcmp(name, "MAX_CONSUMERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Consumers for server ");
		processingMaxConsumers = true;
	} else if (strcmp(name, "ORB_TYPE") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Orb Type for server ");
		processingOrbType = true;
	} else if (strcmp(name, "SERVICE_NAMES") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Service Names for server ");
		processingServiceNames = true;
	} else if (strcmp(name, "SERVICE_NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Service Name for server ");
		processingServiceName = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement(void *userData, const char *name) {
	ServerMetadata* aServerStructPtr = (ServerMetadata*) userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing NAME %s", last_value);
		processingName = false;
		strcpy(server, last_value);
	} else if (strcmp(last_element, "MAX_CHANNELS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxChannels %s", last_value);
		processingMaxChannels = false;
		aServerStructPtr->maxChannels = (short) atol(last_value);
		serverMaxChannels = (short) atol(last_value);
	} else if (strcmp(last_element, "MAX_SUPPLIERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxSuppliers %s", last_value);
		processingMaxSuppliers = false;
		aServerStructPtr->maxSuppliers = (short) atol(last_value);
		serverMaxSuppliers = (short) atol(last_value);
	} else if (strcmp(last_element, "MAX_CONSUMERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxConsumers %s", last_value);
		processingMaxConsumers = false;
		aServerStructPtr->maxConsumers = (short) atol(last_value);
		serverMaxConsumers = (short) atol(last_value);
	} else if (strcmp(last_element, "ORB_TYPE") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing OrbType %s", last_value);
		processingOrbType = false;
		aServerStructPtr->orbType = strdup(last_value);
	} else if (strcmp(last_element, "SERVICE_NAMES") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing ServiceNames ");
		processingServiceNames = false;
	} else if (strcmp(last_element, "SERVICE_NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing ServiceName '%s' at index %d", last_value, serviceNameCount);
		processingServiceNames = false;
		serviceNameCount++;
		//	aServerStructPtr->serviceNames.length(serviceNameCount);
		aServerStructPtr->serviceNames.push_back(strdup(last_value));
	}
	depth -= 1;
}

static void XMLCALL characterData(void *userData, const char *cdata, int len) {
	int i = 0;
	int j = 0;
	int priorLength = strlen(value);

	i = priorLength;
	for (; i < len + priorLength; i++, j++) {
		value[i] = cdata[j];
	}
	value[i] = '\0';
}

bool AtmiBrokerServerXml::parseXmlDescriptor(ServerMetadata* aServerStructPtr,
		const char * aDescriptorFileName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "in parseXmlDescriptor() %s", aDescriptorFileName);

	char  schemaPath[256];
	char* schemaDir;
	bool  result = false;

	schemaDir = ACE_OS::getenv("ATMIBROKER_SCHEMA_DIR");
	if(schemaDir) {
		ACE_OS::snprintf(schemaPath, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"Server.xsd", schemaDir);
	} else {
		ACE_OS::strcpy(schemaPath, "Server.xsd");
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "schemaPath %s", schemaPath);

	XsdValidator validator;
	if(validator.validate(schemaPath, aDescriptorFileName) == false) {
		return false;
	}

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "read file %p", aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServerXml,
				(char*) "loadfile: fstat failed on %s", aDescriptorFileName);
		return false;
	}
	if (s.st_size == 0) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServerXml,
				(char*) "loadfile: file %s is empty", aDescriptorFileName);
		return false;
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "loadfile: file %s is %d long", aDescriptorFileName,
			s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(
				log4cxx::Level::getError(),
				loggerAtmiBrokerServerXml,
				(char*) "loadfile: Could not allocate enough memory to load file %s",
				aDescriptorFileName);
		return false;
	}
	memset(buf, '\0', s.st_size);
	//memcpy(buf,'\0',s.st_size);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "loadfile: Allocated enough memory to load file %d",
			s.st_size);

	//char buf[1024];
	XML_Parser parser = XML_ParserCreate(NULL);
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "created parser");

	int done;
	strcpy(element, "");
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "strcpy /" << element << "/");

	strcpy(value, "");
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "strcpy /" << value << "/");

	XML_SetUserData(parser, aServerStructPtr);
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "set user data" << aServerStructPtr);

	XML_SetElementHandler(parser, startElement, endElement);
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "set element data");

	XML_SetCharacterDataHandler(parser, characterData);
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "set character data");

	try {
		do {
			LOG4CXX_TRACE(loggerAtmiBrokerServerXml, (char*) "reading data" << buf);
			size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
			LOG4CXX_TRACE(loggerAtmiBrokerServerXml, (char*) "read data" << buf);
			done = len < sizeof(buf);
			if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
				userlog(log4cxx::Level::getError(), loggerAtmiBrokerServerXml,
						(char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(
								parser)), XML_GetCurrentLineNumber(parser));
				break;
			}
		} while (!done);
		result = true;
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServerXml, "count not load " << aDescriptorFileName);
	}
	LOG4CXX_TRACE(loggerAtmiBrokerServerXml, "freeing buf");
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServerXml,
			(char*) "leaving parseXmlDescriptor() %s", aDescriptorFileName);
	return result;
}

