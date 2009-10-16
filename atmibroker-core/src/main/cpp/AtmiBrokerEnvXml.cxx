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

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerServiceXml.h"
#include "XsdValidator.h"

#include "log4cxx/logger.h"
#include "ace/ACE.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"

log4cxx::LoggerPtr loggerAtmiBrokerEnvXml(log4cxx::Logger::getLogger(
		"AtmiBrokerEnvXml"));
xarm_config_t * xarmp = 0;
ServersInfo servers;
Buffers buffers;

OrbConfig orbConfig;
MqConfig mqConfig;

char domain[30];
char* queue_name;
char* transFactoryId;

static char last_element[50];
static char last_value[1024];

static char element[50];
static char value[1024];

static int depth = 0;

static int envVariableCount = 0;

static bool processingXaResource = false;
static bool processingEnvVariable = false;
static char* currentBufferName = NULL;
static char* configuration = NULL;

static int CHAR_SIZE = sizeof(char);//1;
static int LONG_SIZE = sizeof(long);//8;
static int INT_SIZE = sizeof(int);//4;
static int SHORT_SIZE = sizeof(short);//2;
static int FLOAT_SIZE = sizeof(float);//INT_SIZE;
static int DOUBLE_SIZE = sizeof(double);//LONG_SIZE;


AtmiBrokerEnvXml::AtmiBrokerEnvXml() {
	depth = 0;
	envVariableCount = 0;

	processingXaResource = false;
	processingEnvVariable = false;
	currentBufferName = NULL;
	configuration = NULL;
}

AtmiBrokerEnvXml::~AtmiBrokerEnvXml() {
}

static int warnCnt = 0;
static void warn(const char * reason) {
	if (warnCnt++ == 0)
		LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) reason);
}

/**
 * Duplicate a value. If the value contains an expression of the for ${ENV}
 * then ENV is interpreted as an environment variable and ${ENV} is replaced
 * by its value (if ENV is not set it is replaced by null string).
 *
 * WARNING: only the first such occurence is expanded. TODO generalise the function
 */
static char * XMLCALL copy_value(const char *value) {
	char *s = (char *) strchr(value, '$');
	char *e;

	if (s && *(s + 1) == '{' && (e = (char *) strchr(s, '}'))) {
		size_t esz = e - s - 2;
		char *en = ACE::strndup(s + 2, esz);
		char *ev = ACE_OS::getenv(en); /* ACE_OS::getenv(en);*/
		char *pr = ACE::strndup(value, (s - value));
		size_t rsz;
		char *v;

		if (ev == NULL) {
			LOG4CXX_WARN(loggerAtmiBrokerEnvXml, (char*) "env variable is unset: " << en);
			ev = (char *) "";
		}

		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char *) "expanding env: "
				<< (s + 2) << (char *) " and e=" << e << (char *) " and en="
				<< en << (char *) " and pr=" << pr << (char *) " and ev=" << ev);
		e += 1;
		rsz = ACE_OS::strlen(pr) + ACE_OS::strlen(e) + ACE_OS::strlen(ev) + 1; /* add 1 for null terminator */
		v = (char *) malloc(rsz);

		ACE_OS::snprintf(v, rsz, "%s%s%s", pr, ev, e);
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, value << (char*) " -> " << v);

		free(en);
		free(pr);

		return v;
	}

	return strdup(value);
}

static bool applicable_config(char *config, const char *attribute) {
	if (config == NULL || ACE_OS::strlen(config) == 0) {
		// see if it is set in the environment
		if ((config = ACE_OS::getenv("BLACKTIE_CONFIGURATION")) == 0)
			return false;
	}

	char * conf = copy_value(attribute);
	bool rtn = strcmp(conf, config);

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "comparing " << conf
			<< " with " << config);
	free(conf);

	return (rtn == 0);
}

static void XMLCALL startElement
(void *userData, const char *name, const char **atts) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing element " << name);
	if (strcmp(name, "ENVIRONMENT xmnls") == 0 || strcmp(name, "ENVIRONMENT") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "starting to read");
	} else if (strcmp(name, "ORB") == 0) {
		for(int i = 0; atts[i]; i += 2) {
			if(strcmp(atts[i], "OPT") == 0) {
				orbConfig.opt = copy_value(atts[i+1]);
				LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "set opt: " << orbConfig.opt);
			} else if(strcmp(atts[i], "TRANS_FACTORY_ID") == 0) {
				orbConfig.transactionFactoryName = copy_value(atts[i+1]);
				LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "set tFN: " << orbConfig.transactionFactoryName);
			}
		}
	} else if (strcmp(name, "MQ") == 0) {
		for(int i = 0; atts[i]; i += 2) {
			if(strcmp(atts[i], "HOST") == 0) {
				mqConfig.host = copy_value(atts[i+1]);
			} else if(strcmp(atts[i], "PORT") == 0) {
				mqConfig.port = atoi(atts[i+1]);
			} else if(strcmp(atts[i], "USER") == 0) {
				mqConfig.user = copy_value(atts[i+1]);
			} else if(strcmp(atts[i], "PASSWORD") == 0) {
				mqConfig.pwd = copy_value(atts[i+1]);
			} else if(strcmp(atts[i], "DESTINATION_TIMEOUT") == 0) {
				mqConfig.destinationTimeout = atoi(atts[i+1]);
			} else if(strcmp(atts[i], "RECEIVE_TIMEOUT") == 0) {
				mqConfig.requestTimeout = atoi(atts[i+1]);
			} else if(strcmp(atts[i], "TIME_TO_LIVE") == 0) {
				mqConfig.timeToLive = atoi(atts[i+1]);
			}
		}
	} else if (strcmp(name, "SERVER") == 0) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing SERVER");

		ServerInfo* server = new ServerInfo;
		if(atts != 0) {
			if(atts[0] && strcmp(atts[0], "name") == 0) {
				server->serverName = copy_value(atts[1]);
			} else {
				server->serverName = copy_value("default");
			}
		}

		servers.push_back(server);
	} else if (strcmp(name, "XA_RESOURCE") == 0) {
		if(strcmp(atts[0], "configuration") == 0 && applicable_config(configuration, atts[1])) {

			LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing xaresource");
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
		} else {
			LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "CONFIGURATION NOT APPLICABLE FOR XA_RESOURCE: " << atts[1]);
		}
	} else if (strcmp(name, "ENV_VARIABLE") == 0) {
		if(atts != 0 && atts[0] && strcmp(atts[0], "configuration") == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "comparing" << atts[1] << " with " << configuration);
			if (strcmp(atts[1], configuration) == 0) {
				LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "processing ENV_VARIABLE");
				processingEnvVariable = true;
			} else {
				LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "CONFIGURATION NOT APPLICABLE FOR ENV_VARIABLE: " << atts[1]);
			}
		} else {
			processingEnvVariable = true;
		}

		if (processingEnvVariable) {
			envVariableCount++;
			envVar_t envVar;
			(*aEnvironmentStructPtr).push_back(envVar);
		}
	} else if (strcmp(name, "BUFFER") == 0) {
		currentBufferName = copy_value(atts[1]);
		Buffer* buffer = buffers[currentBufferName];
		if (buffer == NULL) {
			Buffer* buffer = new Buffer();
			buffer->name = currentBufferName;
			buffer->wireSize = 0;
			buffer->memSize = 0;
			buffers[buffer->name] = buffer;
		} else {
			LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) "Duplicate buffer detected: " << currentBufferName);
		}
	} else if (strcmp(name, "ATTRIBUTE") == 0) {
		if (currentBufferName != NULL) {
			Attribute* attribute = new Attribute();
			attribute->id = NULL;
			attribute->type = NULL;
			attribute->count = 0;
			attribute->length = 0;
			attribute->defaultValue = NULL;
			attribute->wirePosition = 0;
			attribute->memPosition = 0;
			for(int i = 0; atts[i]; i += 2) {
				if(strcmp(atts[i], "id") == 0) {
					attribute->id = copy_value(atts[i+1]);
				} else if(strcmp(atts[i], "type") == 0) {
					attribute->type = copy_value(atts[i+1]);
				} else if(strcmp(atts[i], "count") == 0) {
					attribute->count = atoi(atts[i+1]);
				} else if(strcmp(atts[i], "length") == 0) {
					attribute->length = atoi(atts[i+1]);
				} else if(strcmp(atts[i], "default") == 0) {
					attribute->defaultValue = copy_value(atts[i+1]);
				}
			}
			attribute->wirePosition = buffers[currentBufferName]->wireSize;

			int typeSize = -1;
			int instanceSize = -1;
			Attribute* toCheck = buffers[currentBufferName]->attributes[attribute->id];
			if (toCheck == NULL) {
				buffers[currentBufferName]->attributes[attribute->id] = attribute;
				// short, int, long, float, double, char, char*
				if (strcmp(attribute->type, "short") == 0) {
					typeSize = SHORT_SIZE;
					instanceSize = SHORT_SIZE;
				} else if (strcmp(attribute->type, "int") == 0) {
					typeSize = INT_SIZE;
					instanceSize = INT_SIZE;
				} else if (strcmp(attribute->type, "long") == 0) {
					typeSize = LONG_SIZE;
					instanceSize = LONG_SIZE;
				} else if (strcmp(attribute->type, "float") == 0) {
					typeSize = FLOAT_SIZE;
					instanceSize = FLOAT_SIZE;
				} else if (strcmp(attribute->type, "double") == 0) {
					typeSize = DOUBLE_SIZE;
					instanceSize = DOUBLE_SIZE;
				} else if (strcmp(attribute->type, "char") == 0) {
					typeSize = CHAR_SIZE;
					instanceSize = CHAR_SIZE;
				} else if (strcmp(attribute->type, "char*") == 0) {
					typeSize = CHAR_SIZE;
					instanceSize = CHAR_SIZE * attribute->length;
				} else if (strcmp(attribute->type, "short[]") == 0) {
					typeSize = SHORT_SIZE;
					instanceSize = SHORT_SIZE * attribute->count;
				} else if (strcmp(attribute->type, "int[]") == 0) {
					typeSize = INT_SIZE;
					instanceSize = INT_SIZE * attribute->count;
				} else if (strcmp(attribute->type, "long[]") == 0) {
					typeSize = LONG_SIZE;
					instanceSize = LONG_SIZE * attribute->count;
				} else if (strcmp(attribute->type, "float[]") == 0) {
					typeSize = FLOAT_SIZE;
					instanceSize = FLOAT_SIZE * attribute->count;
				} else if (strcmp(attribute->type, "double[]") == 0) {
					typeSize = DOUBLE_SIZE;
					instanceSize = DOUBLE_SIZE * attribute->count;
				} else if (strcmp(attribute->type, "char[]") == 0) {
					typeSize = CHAR_SIZE;
					instanceSize = CHAR_SIZE * attribute->length;
				} else if (strcmp(attribute->type, "char*[]") == 0) {
					typeSize = CHAR_SIZE;
					instanceSize = CHAR_SIZE * attribute->length * attribute->count;
				}
				buffers[currentBufferName]->memSize = buffers[currentBufferName]->memSize + (buffers[currentBufferName]->memSize % typeSize);
				attribute->memPosition = buffers[currentBufferName]->memSize;
				buffers[currentBufferName]->wireSize = buffers[currentBufferName]->wireSize + instanceSize;
				buffers[currentBufferName]->memSize = buffers[currentBufferName]->memSize + instanceSize;
			} else {
				LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) "Duplicate attribute detected: " << attribute->id);
				free(attribute->id);
				free(attribute->type);
				free(attribute->defaultValue);
				delete attribute;
			}
		} else {
			LOG4CXX_ERROR(loggerAtmiBrokerEnvXml, (char*) "No buffer is being processed");
		}
	} else if(strcmp(name, "SERVICE") == 0 ||
			strcmp(name, "ADMIN_SERVICE") == 0) {

		if(atts != 0) {
			ServiceInfo service;
			service.transportLib = NULL;
			char* server;

			memset(&service, 0, sizeof(ServiceInfo));
			server = servers.back()->serverName;

			if(strcmp(name, "ADMIN_SERVICE") == 0 ) {
				char adm[16];
				ACE_OS::snprintf(adm, 15, "%s_ADMIN", server);
				service.serviceName = copy_value(adm);
			}

			for(int i = 0; atts[i]; i += 2) {
				if(strcmp(atts[i], "name") == 0) {
					service.serviceName = copy_value(atts[i+1]);
					LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "set name: " << service.serviceName);
				}
			}
			LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "setting transportlib");
#ifdef WIN32
			service.transportLib = strdup("atmibroker-hybrid.dll");
#else
			service.transportLib = strdup("libatmibroker-hybrid.so");
#endif
			LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "set transportlib: " << service.transportLib);

			char* dir = AtmiBrokerEnv::ENVIRONMENT_DIR;
			if(dir == NULL) {
				dir = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");
			}
			char configDir[256];

			if(dir != NULL) {
				ACE_OS::snprintf(configDir, 255, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"%s", dir, server);
			} else {
				ACE_OS::strncpy(configDir, server, 255);
			}

			service.advertised = false;
			service.poolSize = 1;
			AtmiBrokerServiceXml xml;
			xml.parseXmlDescriptor(&service, service.serviceName, configDir, configuration);

			if(service.function_name == NULL) {
				service.function_name = copy_value(service.serviceName);
			}
			servers.back()->serviceVector.push_back(service);
		}
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
		strcpy(domain, last_value);
	} else if (strcmp(last_element, "XA_RESOURCE") == 0) {
		processingXaResource = false;
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		if (processingXaResource) {
			xarmp->resourceMgrId = atol(last_value);
		}
	} else if (strcmp(last_element, "XA_RESOURCE_NAME") == 0) {
		if (processingXaResource) {
			xarmp->resourceName = copy_value(last_value);
		}
	} else if (strcmp(last_element, "XA_OPEN_STRING") == 0) {
		if (processingXaResource) {
			xarmp->openString = copy_value(last_value);
		}
	} else if (strcmp(last_element, "XA_CLOSE_STRING") == 0) {
		if (processingXaResource) {
			xarmp->closeString = copy_value(last_value);
		}
	} else if (strcmp(last_element, "XA_SWITCH") == 0) {
		if (processingXaResource) {
			xarmp->xasw = copy_value(last_value);
		}
	} else if (strcmp(last_element, "XA_LIB_NAME") == 0) {
		if (processingXaResource) {
			xarmp->xalib = copy_value(last_value);
		}
	} else if (strcmp(last_element, "ENV_VARIABLE") == 0) {
		if (processingEnvVariable) {
			int index = envVariableCount - 1;
			LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "stored EnvVariable at index %d" << index);
		}
		processingEnvVariable = false;
	} else if (strcmp(last_element, "NAME") == 0) {
		if (processingEnvVariable) {
			int index = envVariableCount - 1;
			(*aEnvironmentStructPtr)[index].name = copy_value(last_value);
			LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "stored EnvName %s at index %d" << last_value << index);
		}
	} else if (strcmp(last_element, "VALUE") == 0) {
		if (processingEnvVariable) {
			int index = envVariableCount - 1;
			(*aEnvironmentStructPtr)[index].value = copy_value(last_value);
			LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "stored Env Value %s at index %d" << last_value << index);
		}
	} else if (strcmp(last_element, "BUFFER") == 0) {
		currentBufferName = NULL;
	}
	depth -= 1;
}

static void XMLCALL characterData
(void *userData, const char *cdata, int len) {
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
		const char * configurationDir, char * conf) {

	char aDescriptorFileName[256];

	if (configurationDir != NULL) {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml, (char*) "read env from dir: "
				<< configurationDir);
		ACE_OS::snprintf(aDescriptorFileName, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"Environment.xml",
				configurationDir);
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml,
				(char*) "in parseXmlDescriptor() " << aDescriptorFileName);
	} else {
		LOG4CXX_TRACE(loggerAtmiBrokerEnvXml,
				(char*) "read env from default file");
		ACE_OS::strcpy(aDescriptorFileName, "Environment.xml");
	}
	configuration = conf;

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, "BLACKTIE_CONFIGURATION: " << configuration);

	bool toReturn = true;
	char schemaPath[256];
	char* schemaDir;

	schemaDir = ACE_OS::getenv("BLACKTIE_SCHEMA_DIR");
	if (schemaDir) {
		ACE_OS::snprintf(schemaPath, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"Environment.xsd", schemaDir);
	} else {
		ACE_OS::strcpy(schemaPath, "Environment.xsd");
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "schemaPath is "
			<< schemaPath);

	XsdValidator validator;
	if (validator.validate(schemaPath, aDescriptorFileName) == false) {
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

