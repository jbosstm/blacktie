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
#include "AtmiBrokerEnv.h"
#include "XsdValidator.h"
#include "userlog.h"
#include "log4cxx/logger.h"
#include "ace/ACE.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"

log4cxx::LoggerPtr loggerAtmiBrokerServiceXml(log4cxx::Logger::getLogger(
		"AtmiBrokerServiceXml"));

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static bool processingService = false;
static bool processingPoolSize = false;

static char* configuration;

AtmiBrokerServiceXml::AtmiBrokerServiceXml() {
}

AtmiBrokerServiceXml::~AtmiBrokerServiceXml() {
}

static void XMLCALL startElement(void *userData, const char *name, const char **atts) {
	ServiceInfo* aServiceStructPtr = (ServiceInfo*) userData;

	if (strcmp(name, "SERVICE ") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "start element SERVICE");
		processingService = true;
	} else if (strcmp(name, "SIZE") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "processing MAX Cache for service ");
		processingPoolSize = true;
	} else if (strcmp(name, "LIBRARY_NAME") == 0) {
		if(atts != 0 && atts[0] && strcmp(atts[0], "configuration") == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "comparing" << atts[1] << " with " << configuration);
			if (strcmp(atts[1], configuration) == 0) {
				aServiceStructPtr->library_name = strdup(atts[3]);
				LOG4CXX_TRACE(loggerAtmiBrokerServiceXml, (char*) "processed LIBRARY_NAME: " << aServiceStructPtr->library_name);
			} else {
				LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "CONFIGURATION NOT APPLICABLE FOR LIBRARY_NAME: " << atts[1]);
			}
		}
	} else if(strcmp(name, "SERVICE_DESCRIPTION") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "processing SERVICE DESCRIPTION");
		if(atts != 0) {
			for(int i = 0; atts[i]; i += 2) {
				if(strcmp(atts[i], "function_name") == 0) {
					aServiceStructPtr->function_name = strdup(atts[i+1]);
				} else if(strcmp(atts[i], "advertised") == 0) {
					if(strcmp(atts[i+1], "true") == 0) {
						aServiceStructPtr->advertised = true;
					} else {
						aServiceStructPtr->advertised = false;
					}
				}
			}
		}
	}
	ACE_OS::strncpy(element, name, 50);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement(void *userData, const char *name) {
	ServiceInfo* aServiceStructPtr = (ServiceInfo*) userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "SIZE") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "storing MaxCache " << last_value);
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

void AtmiBrokerServiceXml::parseXmlDescriptor(ServiceInfo* aServiceStructPtr,
		const char * aDescriptorFileName, const char * ConfigurationDir, char* conf) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml,
			(char*) "in parseXmlDescriptor() " << aDescriptorFileName);

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	char* serviceConfigFilename =
			(char*) malloc(XATMI_SERVICE_NAME_LENGTH + 10); // TODO this is the length of the service + 10
	memset(serviceConfigFilename, '\0', XATMI_SERVICE_NAME_LENGTH + 10);
	strncpy(serviceConfigFilename, aDescriptorFileName,
			XATMI_SERVICE_NAME_LENGTH); // TODO this is the length of the service
	strcat(serviceConfigFilename, ".xml");

	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "loading: "
			<< serviceConfigFilename);
	char configPath[256];
	memset(configPath, '\0', 256);
	if (ConfigurationDir != NULL) {
		ACE_OS::snprintf(configPath, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"%s", ConfigurationDir, serviceConfigFilename);
	} else {
		ACE_OS::strncpy(configPath, serviceConfigFilename, 256);
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "configPath is : "
			<< configPath);

	char schemaPath[256];
	char* schemaDir;

	schemaDir = ACE_OS::getenv("BLACKTIE_SCHEMA_DIR");
	if (schemaDir) {
		ACE_OS::snprintf(schemaPath, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"Service.xsd", schemaDir);
	} else {
		ACE_OS::strcpy(schemaPath, "Service.xsd");
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "schemaPath "
			<< schemaPath);

	XsdValidator validator;
	if (validator.validate(schemaPath, configPath) == false) {
		free(serviceConfigFilename);
		return;
	}

	if (conf == NULL) {
		LOG4CXX_ERROR(loggerAtmiBrokerServiceXml,
				(char*) "server was started with -c <configuration> unspecified and service configured: " << aDescriptorFileName);
		throw new std::exception();
	}
	configuration = conf;


	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(configPath, "r");

	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "read file "
			<< aDescriptorFile);

	if (!aDescriptorFile) {
		LOG4CXX_WARN(loggerAtmiBrokerServiceXml,
				(char*) "parseXmlDescriptor could not load service config"
						<< serviceConfigFilename);
		free(serviceConfigFilename);
		return;
	}
	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		LOG4CXX_ERROR(loggerAtmiBrokerServiceXml,
				(char*) "loadfile: fstat failed on: " << serviceConfigFilename);
	}
	if (s.st_size == 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerServiceXml,
				(char*) "loadfile: file is empty: " << serviceConfigFilename);
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml, (char*) "loadfile: file "
			<< serviceConfigFilename << " is SIZE " << s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size + 1);
	if (!buf) {
		LOG4CXX_ERROR(
				loggerAtmiBrokerServiceXml,
				(char*) "loadfile: Could not allocate enough memory to load file: "
						<< serviceConfigFilename);
	}

	memset(buf, '\0', s.st_size + 1);
	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml,
			(char*) "loadfile: Allocated enough memory to load file: "
					<< s.st_size);

	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	strcpy(element, "");
	strcpy(value, "");
	XML_SetUserData(parser, aServiceStructPtr);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	do {
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		done = len < sizeof(buf);
		if (len > 0) {
			LOG4CXX_TRACE(loggerAtmiBrokerServiceXml, (char*) "buf is " << buf);
			if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
				LOG4CXX_ERROR(loggerAtmiBrokerServiceXml, (char*) "Error: "
						<< XML_ErrorString(XML_GetErrorCode(parser)) << " at line "
						<< XML_GetCurrentLineNumber(parser));
				break;
			}
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);

	if (aServiceStructPtr->function_name == NULL) {
		aServiceStructPtr->function_name = strdup(
				aServiceStructPtr->serviceName);
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServiceXml,
			(char*) "leaving parseXmlDescriptor() " << serviceConfigFilename);
	free(serviceConfigFilename);
}
