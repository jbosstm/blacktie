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
#include <stdexcept>

#include "expat.h"

#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerServiceXml.h"
#include "XsdValidator.h"

#include "log4cxx/logger.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"

log4cxx::LoggerPtr loggerAtmiBrokerEnvXml(log4cxx::Logger::getLogger(
		"AtmiBrokerEnvXml"));

// environment will be free in ~AtmiBrokerEnv
xarm_config_t * xarmp = 0;
ServersInfo servers;

char* notifyServiceId;
char* namingServiceId;
char* loggingServiceId;
char domain[30];
char* queue_name;
char* transFactoryId;

static char last_element[50];
static char last_value[1024];

static char element[50];
static char value[1024];

static int depth = 0;

static int envVariableCount = 0;

static bool processingEnvironment = false;
static bool processingEnvironmentDescription = false;
static bool processingServer = false;
static bool processingQSpaceName = false;
static bool processingNamingServiceId = false;
static bool processingNotifyServiceId = false;
static bool processingLoggingServiceId = false;
static bool processingTransFactoryId = false;
static bool processingServerNames = false;
static bool processingServiceNames = false;
static bool processingService = false;
static bool processingXaResources = false;
static bool processingXaResource = false;
static bool processingXaResourceMgrId = false;
static bool processingXaResourceName = false;
static bool processingXaOpenString = false;
static bool processingXaCloseString = false;
static bool processingXaSwitch = false;
static bool processingXaLibName = false;
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
	processingEnvironmentDescription = false;
	processingServer = false;
	processingQSpaceName = false;
	processingNamingServiceId = false;
	processingNotifyServiceId = false;
	processingLoggingServiceId = false;
	processingTransFactoryId = false;
	processingXaResources = false;
	processingXaResource = false;
	processingXaResourceMgrId = false;
	processingXaResourceName = false;
	processingXaOpenString = false;
	processingXaCloseString = false;
	processingXaSwitch = false;
	processingXaLibName = false;
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

static int warnCnt = 0;
static void warn(const char * reason) {
	if (warnCnt++ == 0)
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) reason);
	//std::invalid_argument ex(reason);
	//throw ex;
}

static void XMLCALL startElement
(void *userData, const char *name, const char **atts) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing element " << name);

	if (strcmp(name, "ENVIRONMENT xmnls") == 0 || strcmp(name, "ENVIRONMENT") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "starting to read");
		processingEnvironment = true;
	} else if (strcmp(name, "ENVIRONMENT_DESCRIPTION") == 0) {
		processingEnvironmentDescription = true;
	} else if (strcmp(name, "SERVER") == 0) {
		processingServer = true;

		ServerInfo* server = (ServerInfo*) malloc (sizeof(ServerInfo));
		memset(server, 0, sizeof(ServerInfo));
		if(atts != 0) {
			if(atts[0] && strcmp(atts[0], "name") == 0) {
				server->serverName = strdup(atts[1]);
			} else {
				server->serverName = strdup("default");
			}
		}

		servers.push_back(server);
	} else if (strcmp(name, "DOMAIN") == 0) {
		processingDomain = true;
	} else if (strcmp(name, "QSPACE_NAME") == 0) {
		processingQSpaceName = true;
	} else if (strcmp(name, "NAMING_SERVICE_ID") == 0) {
		processingNamingServiceId = true;
	} else if (strcmp(name, "NOTIFY_SERVICE_ID") == 0) {
		processingNotifyServiceId = true;
	} else if (strcmp(name, "LOGGING_SERVICE_ID") == 0) {
		processingLoggingServiceId = true;
	} else if (strcmp(name, "TRANS_FACTORY_ID") == 0) {
		processingTransFactoryId = true;
	} else if (strcmp(name, "XA_RESOURCES") == 0) {
		processingXaResources = true;
	} else if (strcmp(name, "XA_RESOURCE") == 0) {
		processingXaResource = true;
		xarm_config_t *p;
		if ((p = (xarm_config_t *) malloc(sizeof(xarm_config_t))) == 0) {
			warnCnt = 0;
			warn("out of memory");
		} else {
			(void *) memset(p, 0, sizeof(xarm_config_t));

			if (xarmp == 0) {
				p->head = p;
			} else {
				xarmp->next = p;
				p->head = xarmp->head;
			}

			xarmp = p;
		}
	} else if (strcmp(name, "XA_RESOURCE_MGR_ID") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaResourceMgrId = true;
	} else if (strcmp(name, "XA_RESOURCE_NAME") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaResourceName = true;
	} else if (strcmp(name, "XA_OPEN_STRING") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaOpenString = true;
	} else if (strcmp(name, "XA_CLOSE_STRING") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaCloseString = true;
	} else if (strcmp(name, "XA_SWITCH") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaSwitch = true;
	} else if (strcmp(name, "XA_LIB_NAME") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaLibName = true;
	} else if (strcmp(name, "XA_THREAD_MODEL") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaThreadModel = true;
	} else if (strcmp(name, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaAutomaticAssociation = true;
	} else if (strcmp(name, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		if (xarmp == 0) warn("XML document error: XA_RESOURCES element is invalid");
		processingXaDynamicRegistrationOptimization = true;
	} else if (strcmp(name, "ENV_VARIABLES") == 0) {
		processingEnvVariables = true;
	} else if (strcmp(name, "ENV_VARIABLE") == 0) {
		processingEnvVariable = true;
		envVariableCount++;
		envVar_t envVar;
		(*aEnvironmentStructPtr).push_back(envVar);
	} else if (strcmp(name, "NAME") == 0) {
		processingEnvName = true;
	} else if (strcmp(name, "VALUE") == 0) {
		processingEnvValue = true;
	} else if (strcmp(name, "SERVER_NAMES") == 0) {
		processingServerNames = true;
	} else if(strcmp(name, "SERVICE_NAMES") == 0) {
		processingServiceNames = true;
	} else if(strcmp(name, "SERVICE") == 0) {
		processingService = true;

		if(atts != 0) {
			ServiceInfo service;
			char*       server;

			memset(&service, 0, sizeof(ServiceInfo));
			for(int i = 0; atts[i]; i += 2) {
				if(strcmp(atts[i], "name") == 0) {
					service.serviceName = strdup(atts[i+1]);
				} else if(strcmp(atts[i], "transportLibrary") == 0) {
					service.transportLib = strdup(atts[i+1]);
				}
			}
			server = servers.back()->serverName;
			char* dir = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");
			char configDir[256];

			if(dir != NULL) {
				ACE_OS::snprintf(configDir, 255, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"%s", dir, server);
			} else {
				ACE_OS::strncpy(configDir, server, 255);
			}

			service.advertised = false;
			service.poolSize = 1;
			AtmiBrokerServiceXml xml;
			xml.parseXmlDescriptor(&service, service.serviceName, configDir);

			if(service.function_name == NULL) {
				service.function_name = strdup(service.serviceName);
			}
			servers.back()->serviceVector.push_back(service);
		}
	} else {
		LOG4CXX_WARN(loggerAtmiBrokerEnvXml, (char*) "unrecognized environment var: " << (char*) name);
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement
(void *userData, const char *name) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	bool storedElement = false;
	strcpy(last_element, name);
	strcpy(last_value, value);

	LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "storing element " << last_element << (char *) " value=" << last_value);

	if (strcmp(last_element, "DOMAIN") == 0) {
		storedElement = true;
		processingDomain = false;
		strcpy(domain, last_value);
	} else if (strcmp(last_element, "QSPACE_NAME") == 0) {
		storedElement = true;
		processingQSpaceName = false;
		queue_name = strdup(last_value);
	} else if (strcmp(last_element, "NAMING_SERVICE_ID") == 0) {
		processingNamingServiceId = false;
		namingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "NOTIFY_SERVICE_ID") == 0) {
		processingNotifyServiceId = false;
		notifyServiceId = strdup(last_value);
	} else if (strcmp(last_element, "LOGGING_SERVICE_ID") == 0) {
		processingLoggingServiceId = false;
		loggingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "TRANS_FACTORY_ID") == 0) {
		processingTransFactoryId = false;
		transFactoryId = strdup(last_value);
	} else if (strcmp(last_element, "XA_RESOURCES") == 0) {
		processingXaResources = false;
	} else if (strcmp(last_element, "XA_RESOURCE") == 0) {
		processingXaResource = false;
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		processingXaResourceMgrId = false;
		if (xarmp) xarmp->resourceMgrId = atol(last_value);
	} else if (strcmp(last_element, "XA_RESOURCE_NAME") == 0) {
		processingXaResourceName = false;
		if (xarmp) xarmp->resourceName = strdup(last_value);
	} else if (strcmp(last_element, "XA_OPEN_STRING") == 0) {
		processingXaOpenString = false;
		if (xarmp) xarmp->openString = strdup(last_value);
	} else if (strcmp(last_element, "XA_CLOSE_STRING") == 0) {
		processingXaCloseString = false;
		if (xarmp) xarmp->closeString = strdup(last_value);
	} else if (strcmp(last_element, "XA_SWITCH") == 0) {
		processingXaSwitch = false;
		if (xarmp) xarmp->xasw = strdup(last_value);
	} else if (strcmp(last_element, "XA_LIB_NAME") == 0) {
		processingXaLibName = false;
		if (xarmp) xarmp->xalib = strdup(last_value);
	} else if (strcmp(last_element, "XA_THREAD_MODEL") == 0) {
		processingXaThreadModel = false;
		if (xarmp) xarmp->threadModel = (strcmp(last_value, "TRUE") == 0);
	} else if (strcmp(last_element, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		processingXaAutomaticAssociation = false;
		if (xarmp) xarmp->automaticAssociation = (strcmp(last_value, "TRUE") == 0);
	} else if (strcmp(last_element, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		processingXaDynamicRegistrationOptimization = false;
		if (xarmp) xarmp->dynamicRegistrationOptimization = (strcmp(last_value, "TRUE") == 0);
	} else if (strcmp(last_element, "ENV_VARIABLES") == 0) {
		processingEnvVariables = false;
	} else if (strcmp(last_element, "ENV_VARIABLE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "\tstored EnvVariable at index %d" << index);
		processingEnvVariable = false;
	} else if (strcmp(last_element, "NAME") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "\tstoring EnvName %s at index %d" << last_value << index);
		processingEnvName = false;
		(*aEnvironmentStructPtr)[index].name = strdup(last_value);
	} else if (strcmp(last_element, "VALUE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "\tstoring Env Value %s at index %d" << last_value << index);
		processingEnvValue = false;
		(*aEnvironmentStructPtr)[index].value = strdup(last_value);
	} else if(strcmp(last_element, "SERVER_NAMES") == 0) {
		processingServerNames = false;
	} else if(strcmp(last_element, "SERVER") == 0) {
		processingServer = false;
	} else if(strcmp(last_element, "SERVICE_NAMES") == 0) {
		processingServiceNames = false;
	} else if(strcmp(last_element, "SERVICE") == 0) {
		processingService = false;
	}
	depth -= 1;
}

static void XMLCALL characterData
(void *userData, const char *cdata, int len) {
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

bool AtmiBrokerEnvXml::parseXmlDescriptor(
		std::vector<envVar_t>* aEnvironmentStructPtr,
		const char * aDescriptorFileName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "in parseXmlDescriptor() " << aDescriptorFileName);

	bool toReturn = true;
	char  schemaPath[256];
	char* schemaDir;

	schemaDir = ACE_OS::getenv("BLACKTIE_SCHEMA_DIR");
	if(schemaDir) {
		ACE_OS::snprintf(schemaPath, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"Environment.xsd", schemaDir);
	} else {
		ACE_OS::strcpy(schemaPath, "Environment.xsd");
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "schemaPath is " << schemaPath);

	XsdValidator validator;
	if(validator.validate(schemaPath, aDescriptorFileName) == false) {
		return false;
	}
	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	if (!aDescriptorFile) {
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml,
				(char*) "loadfile: fopen failed on %s" << aDescriptorFileName);
		return false;
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "read file %p"
			<< aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml,
				(char*) "loadfile: fstat failed on %s" << aDescriptorFileName);
	}
	if (s.st_size == 0) {
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml,
				(char*) "loadfile: file %s is empty" << aDescriptorFileName);
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml,
			(char*) "loadfile: file %s is %d long" << aDescriptorFileName
					<< s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		LOG4CXX_ERROR(
				loggerAtmiBrokerEnvXml,
				(char*) "loadfile: Could not allocate enough memory to load file %s"
						<< aDescriptorFileName);
	}
	for (unsigned int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml,
			(char*) "loadfile: Allocated enough memory to load file %d"
					<< s.st_size);

	/*
	namingServiceId = strdup("");
	notifyServiceId = strdup("");;
	loggingServiceId = strdup("");
	*/

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
			LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) "%d at line %d"
					<< XML_ErrorString(XML_GetErrorCode(parser))
					<< XML_GetCurrentLineNumber(parser));
			toReturn = false;
			break;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml,
			(char*) "leaving parseXmlDescriptor() %s" << aDescriptorFileName);

	if (warnCnt) {
		warnCnt = 0;
		return false;
	}

	return toReturn;
}

