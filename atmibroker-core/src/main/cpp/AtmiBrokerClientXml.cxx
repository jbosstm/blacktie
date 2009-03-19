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
#include <string.h>
#include <iostream>
#include "expat.h"
#include <sys/stat.h>

#include "AtmiBrokerClientXml.h"
#include "userlog.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerAtmiBrokerClientXml(log4cxx::Logger::getLogger(
		"AtmiBrokerClientXml"));

int clientMaxChannels = 0;
int clientMaxSuppliers = 0;
int clientMaxConsumers = 0;

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static int serviceNameCount = 0;

static bool processingServer = false;
//TODO -NOT IMPLEMENTED static bool processingMaxReplicas		= false;
static bool processingMaxSuppliers = false;
static bool processingMaxConsumers = false;
static bool processingMaxChannels = false;
static bool processingServerNames = false;
static bool processingServerName = false;
static bool processingServiceNames = false;
static bool processingServiceName = false;

ClientServerInfo* aClientServerInfoPtr = NULL;

AtmiBrokerClientXml::AtmiBrokerClientXml() {
}

AtmiBrokerClientXml::~AtmiBrokerClientXml() {
}

static void XMLCALL startElement(void *userData, const char *name, const char **atts) {
	if (strcmp(name, "SERVER xmnls") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "new server ");
		processingServer = true;

	} else if (strcmp(name, "MAX_CHANNELS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Channels for server ");
		processingMaxChannels = true;
	} else if (strcmp(name, "MAX_SUPPLIERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Suppliers for server ");
		processingMaxSuppliers = true;
	} else if (strcmp(name, "MAX_CONSUMERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Consumers for server ");
		processingMaxConsumers = true;
	} else if (strcmp(name, "SERVICE_NAMES") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Service Names for server ");
		processingServiceNames = true;
	} else if (strcmp(name, "SERVICE_NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Service Name for server ");
		processingServiceName = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement(void *userData, const char *name) {
	std::vector<ClientServerInfo*>* aClientServerVectorPtr = (std::vector<ClientServerInfo*>*) userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "MAX_CHANNELS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxChannels %s", last_value);
		processingMaxChannels = false;
		//aClientServerInfoPtr->cInfo->maxChannels = atol(last_value);
		clientMaxChannels = atol(last_value);
	} else if (strcmp(last_element, "MAX_SUPPLIERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxSuppliers %s", last_value);
		processingMaxSuppliers = false;
		//aClientServerInfoPtr->cInfo->maxSuppliers = atol(last_value);
		clientMaxSuppliers = atol(last_value);
	} else if (strcmp(last_element, "MAX_CONSUMERS") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxConsumers %s", last_value);
		processingMaxConsumers = false;
		//aClientServerInfoPtr->cInfo->maxConsumers = atol(last_value);
		clientMaxConsumers = atol(last_value);
	} else if (strcmp(last_element, "SERVER_NAMES") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServerNames ");
		processingServerNames = false;
	} else if (strcmp(last_element, "SERVER_NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServerName '%s'", last_value);
		processingServerName = false;

		aClientServerInfoPtr = (ClientServerInfo*) malloc(sizeof(ClientServerInfo) * 1);
		memset(aClientServerInfoPtr, '\0', sizeof(ClientServerInfo));
		aClientServerInfoPtr->serverName = strdup(last_value);
		aClientServerInfoPtr->serviceVectorPtr = new std::vector<char*>;

		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "adding aClientServerInfo %p to vector", aClientServerInfoPtr);
		aClientServerVectorPtr->push_back(aClientServerInfoPtr);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "added aClientServerInfo %p to vector", aClientServerInfoPtr);

	} else if (strcmp(last_element, "SERVICE_NAMES") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServiceNames ");
		processingServiceNames = false;
	} else if (strcmp(last_element, "SERVICE_NAME") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServiceName '%s'", last_value);
		processingServiceName = false;
		serviceNameCount++;
		aClientServerInfoPtr->serviceVectorPtr->push_back(strdup(last_value));
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

void AtmiBrokerClientXml::parseXmlDescriptor(
		std::vector<ClientServerInfo*>* aClientServerVectorPtr,
		const char * aDescriptorFileName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
			(char*) "parseXmlDescriptor() %s", aDescriptorFileName);

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
			(char*) "read file %p", aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerClientXml,
				(char*) "loadfile: fstat failed on %s", aDescriptorFileName);
	}
	if (s.st_size == 0) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerClientXml,
				(char*) "loadfile: file %s is empty", aDescriptorFileName);
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
			(char*) "loadfile: file %s is %d long", aDescriptorFileName,
			s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(
				log4cxx::Level::getError(),
				loggerAtmiBrokerClientXml,
				(char*) "loadfile: Could not allocate enough memory to load file %s",
				aDescriptorFileName);
	}
	for (unsigned int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
			(char*) "loadfile: Allocated enough memory to load file %d",
			s.st_size);

	//char buf[1024];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	strcpy(element, "");
	strcpy(value, "");
	XML_SetUserData(parser, aClientServerVectorPtr);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	do {
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
				(char*) "buf is %s", buf);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerClientXml,
					(char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(
							parser)), XML_GetCurrentLineNumber(parser));
			break;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClientXml,
			(char*) "leaving parseXmlDescriptor() %s", aDescriptorFileName);
}

