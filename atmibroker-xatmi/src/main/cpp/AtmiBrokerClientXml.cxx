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

#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#endif
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "expat.h"

#include "AtmiBrokerClient.h"
#include "AtmiBrokerClientXml.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerClientXml(Logger::getLogger("AtmiBrokerClientXml"));

int clientMaxChannels = 0;
int clientMaxSuppliers = 0;
int clientMaxConsumers = 0;

const char* AtmiBrokerClientXml::Client_Begin_Tag = (char*) "<?xml version=\"1.0\"?>\n<CLIENT xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";

const char* AtmiBrokerClientXml::Client_End_Tag = (char*) "</CLIENT>\n";

const char* AtmiBrokerClientXml::Client_Desc_Begin_Tag = (char*) "  <CLIENT_DESCRIPTION>\n";
const char* AtmiBrokerClientXml::Client_Desc_End_Tag = (char*) "  </CLIENT_DESCRIPTION>\n";

const char* AtmiBrokerClientXml::Max_Replicas_Begin_Tag = (char*) "    <MAX_REPLICAS>";
const char* AtmiBrokerClientXml::Max_Replicas_End_Tag = (char*) "</MAX_REPLICAS>\n";

const char* AtmiBrokerClientXml::Max_Channels_Begin_Tag = (char*) "    <MAX_CHANNELS>";
const char* AtmiBrokerClientXml::Max_Channels_End_Tag = (char*) "</MAX_CHANNELS\n>";

const char* AtmiBrokerClientXml::Max_Suppliers_Begin_Tag = (char*) "    <MAX_SUPPLIERS>";
const char* AtmiBrokerClientXml::Max_Suppliers_End_Tag = (char*) "</MAX_SUPPLIERS>\n";

const char* AtmiBrokerClientXml::Max_Consumers_Begin_Tag = (char*) "    <MAX_CONSUMERS>";
const char* AtmiBrokerClientXml::Max_Consumers_End_Tag = (char*) "</MAX_CONSUMERS>\n";

const char* AtmiBrokerClientXml::Server_Names_Begin_Tag = (char*) "    <SERVER_NAMES>\n";
const char* AtmiBrokerClientXml::Server_Names_End_Tag = (char*) "    </SERVER_NAMES>\n";

const char* AtmiBrokerClientXml::Server_Name_Begin_Tag = (char*) "      <SERVER_NAME>";
const char* AtmiBrokerClientXml::Server_Name_End_Tag = (char*) "</SERVER_NAME>\n";

const char* AtmiBrokerClientXml::Service_Names_Begin_Tag = (char*) "      <SERVICE_NAMES>\n";
const char* AtmiBrokerClientXml::Service_Names_End_Tag = (char*) "      </SERVICE_NAMES>\n";

const char* AtmiBrokerClientXml::Service_Name_Begin_Tag = (char*) "        <SERVICE_NAME>";
const char* AtmiBrokerClientXml::Service_Name_End_Tag = (char*) "</SERVICE_NAME>\n";

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

const char *
AtmiBrokerClientXml::writeXmlDescriptor(std::vector<ClientServerInfo*>* serverData) {
	userlog(Level::getError(), loggerAtmiBrokerClientXml, (char*) "writeXmlDescriptor() - Not implemented");
	return NULL;
	//return NULL;
}

static void XMLCALL startElement
(void *userData, const char *name, const char **atts)
{
	std::vector<ClientServerInfo*>* aClientServerVectorPtr = (std::vector<ClientServerInfo*>*)userData;

	if (strcmp(name, "SERVER xmnls") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "new server ");
		processingServer = true;

	}
	//TODO -NOT IMPLEMENTED   else if (strcmp(name, "MAX_REPLICAS") == 0)
	//  {
	//	  userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Replicas for server ");
	//  	  processingMaxReplicas = true;
	//  }
	else if (strcmp(name, "MAX_CHANNELS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Channels for server ");
		processingMaxChannels = true;
	}
	else if (strcmp(name, "MAX_SUPPLIERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Suppliers for server ");
		processingMaxSuppliers = true;
	}
	else if (strcmp(name, "MAX_CONSUMERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Max Consumers for server ");
		processingMaxConsumers = true;
	}
	else if (strcmp(name, "SERVICE_NAMES") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Service Names for server ");
		processingServiceNames = true;
	}
	else if (strcmp(name, "SERVICE_NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "processing Service Name for server ");
		processingServiceName = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement
(void *userData, const char *name)
{
	std::vector<ClientServerInfo*>* aClientServerVectorPtr = (std::vector<ClientServerInfo*>*)userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	//TODO -NOT IMPLEMENTED   else if (strcmp(last_element, "MAX_REPLICAS") == 0)
	//  {
	//	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxReplicas %s", last_value);
	//	processingMaxReplicas		= false;
	//	aClientServerInfoPtr->cInfo->maxReplicas = (short)atol(last_value);
	//	maxReplicas = atol(last_value);
	//  }
	if (strcmp(last_element, "MAX_CHANNELS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxChannels %s", last_value);
		processingMaxChannels = false;
		//aClientServerInfoPtr->cInfo->maxChannels = atol(last_value);
		clientMaxChannels = atol(last_value);
	}
	else if (strcmp(last_element, "MAX_SUPPLIERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxSuppliers %s", last_value);
		processingMaxSuppliers = false;
		//aClientServerInfoPtr->cInfo->maxSuppliers = atol(last_value);
		clientMaxSuppliers = atol(last_value);
	}
	else if (strcmp(last_element, "MAX_CONSUMERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing MaxConsumers %s", last_value);
		processingMaxConsumers = false;
		//aClientServerInfoPtr->cInfo->maxConsumers = atol(last_value);
		clientMaxConsumers = atol(last_value);
	}
	else if (strcmp(last_element, "SERVER_NAMES") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServerNames ");
		processingServerNames = false;
	}
	else if (strcmp(last_element, "SERVER_NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServerName '%s'", last_value);
		processingServerName = false;

		aClientServerInfoPtr = (ClientServerInfo*)malloc(sizeof(ClientServerInfo)*1);
		memset(aClientServerInfoPtr, '\0', sizeof(ClientServerInfo));
		aClientServerInfoPtr->serverName = strdup(last_value);
		aClientServerInfoPtr->serviceVectorPtr = new std::vector<char*>;

		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "adding aClientServerInfo %p to vector", aClientServerInfoPtr);
		aClientServerVectorPtr->push_back(aClientServerInfoPtr);
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "added aClientServerInfo %p to vector", aClientServerInfoPtr);

	}
	else if (strcmp(last_element, "SERVICE_NAMES") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServiceNames ");
		processingServiceNames = false;
	}
	else if (strcmp(last_element, "SERVICE_NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "storing ServiceName '%s'", last_value);
		processingServiceName = false;
		serviceNameCount++;
		aClientServerInfoPtr->serviceVectorPtr->push_back(strdup(last_value));
	}
	depth -= 1;
}

static void XMLCALL characterData
(void *userData, const char *cdata, int len)
{
	std::vector<ClientServerInfo*>* aClientServerVectorPtr = (std::vector<ClientServerInfo*>*)userData;

	int i = 0;
	int j = 0;
	int priorLength = strlen(value);

	i = priorLength;
	for (; i < len+priorLength; i++, j++)
	{
		value[i] = cdata[j];
	}
	value[i] = '\0';
}

void AtmiBrokerClientXml::parseXmlDescriptor(std::vector<ClientServerInfo*>* aClientServerVectorPtr, const char * aDescriptorFileName) {
	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "parseXmlDescriptor() %s", aDescriptorFileName);

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "read file %p", aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(Level::getError(), loggerAtmiBrokerClientXml, (char*) "loadfile: fstat failed on %s", aDescriptorFileName);
	}
	if (s.st_size == 0) {
		userlog(Level::getError(), loggerAtmiBrokerClientXml, (char*) "loadfile: file %s is empty", aDescriptorFileName);
	}
	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "loadfile: file %s is %d long", aDescriptorFileName, s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(Level::getError(), loggerAtmiBrokerClientXml, (char*) "loadfile: Could not allocate enough memory to load file %s", aDescriptorFileName);
	}
	for (int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "loadfile: Allocated enough memory to load file %d", s.st_size);

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
		userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "buf is %s", buf);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
			return;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);
	userlog(Level::getDebug(), loggerAtmiBrokerClientXml, (char*) "leaving parseXmlDescriptor() %s", aDescriptorFileName);
}

