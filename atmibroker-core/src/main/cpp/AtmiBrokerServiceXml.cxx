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

#include "AtmiBrokerServiceXml.h"
#include "userlog.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerAtmiBrokerServiceXml(log4cxx::Logger::getLogger(
		"AtmiBrokerServiceXml"));

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static bool processingService = false;
static bool processingPoolSize = false;

AtmiBrokerServiceXml::AtmiBrokerServiceXml() {
}

AtmiBrokerServiceXml::~AtmiBrokerServiceXml() {
}

static void XMLCALL startElement(void *userData, const char *name, const char **atts) {
	if (strcmp(name, "SERVICE ") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "new service ");
		processingService = true;
	} else if (strcmp(name, "SIZE") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "processing MAX Cache for service ");
		processingPoolSize = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement(void *userData, const char *name) {
	SVCINFO* aServiceStructPtr = (SVCINFO*) userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "SIZE") == 0) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "storing MaxCache %s", last_value);
		processingPoolSize = false;
		aServiceStructPtr->poolSize = (short) atol(last_value);
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

void AtmiBrokerServiceXml::parseXmlDescriptor(SVCINFO* aServiceStructPtr,
		const char * aDescriptorFileName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
			(char*) "in parseXmlDescriptor() %s", aDescriptorFileName);

	char* serviceConfigFilename = (char*) malloc(25); // TODO this is the length of the service + 10
	memset(serviceConfigFilename, '\0', 25);
	strncpy(serviceConfigFilename, aDescriptorFileName, 15); // TODO this is the length of the service
	strcat(serviceConfigFilename, ".xml");
	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "loading: "
			<< serviceConfigFilename);

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(serviceConfigFilename, "r");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
			(char*) "read file %p", aDescriptorFile);

	if (!aDescriptorFile) {
		userlog(log4cxx::Level::getWarn(), loggerAtmiBrokerServiceXml,
				(char*) "parseXmlDescriptor could not load service config %s",
				serviceConfigFilename);
		return;
	}
	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServiceXml,
				(char*) "loadfile: fstat failed on %s", serviceConfigFilename);
	}
	if (s.st_size == 0) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServiceXml,
				(char*) "loadfile: file %s is empty", serviceConfigFilename);
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
			(char*) "loadfile: file %s is %d long", serviceConfigFilename,
			s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(
				log4cxx::Level::getError(),
				loggerAtmiBrokerServiceXml,
				(char*) "loadfile: Could not allocate enough memory to load file %s",
				serviceConfigFilename);
	}
	for (unsigned int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
			(char*) "loadfile: Allocated enough memory to load file %d",
			s.st_size);

	//char buf[1024];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	strcpy(element, "");
	strcpy(value, "");
	XML_SetUserData(parser, aServiceStructPtr);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	do {
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
				(char*) "buf is %s", buf);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerServiceXml,
					(char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(
							parser)), XML_GetCurrentLineNumber(parser));
			break;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);

	free(serviceConfigFilename);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceXml,
			(char*) "leaving parseXmlDescriptor() %s", serviceConfigFilename);
}

