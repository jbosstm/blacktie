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

#include "AtmiBrokerEnvXml.h"
#include "AtmiBroker_ServerImpl.h"
#include "AtmiBrokerServerXml.h"
#include "userlog.h"
#include "AtmiBroker.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServerXml(Logger::getLogger("AtmiBrokerServerXml"));

int serverMaxChannels = 0;
int serverMaxSuppliers = 0;
int serverMaxConsumers = 0;

const char* AtmiBrokerServerXml::Server_Begin_Tag = (char*) "<?xml version=\"1.0\"?>\n<SERVER xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";

const char* AtmiBrokerServerXml::Server_End_Tag = (char*) "</SERVER>\n";

const char* AtmiBrokerServerXml::Server_Desc_Begin_Tag = (char*) "  <SERVER_DESCRIPTION>\n";
const char* AtmiBrokerServerXml::Server_Desc_End_Tag = (char*) "  </SERVER_DESCRIPTION>\n";

const char* AtmiBrokerServerXml::Max_Replicas_Begin_Tag = (char*) "    <MAX_REPLICAS>";
const char* AtmiBrokerServerXml::Max_Replicas_End_Tag = (char*) "</MAX_REPLICAS>\n";

const char* AtmiBrokerServerXml::Max_Channels_Begin_Tag = (char*) "    <MAX_CHANNELS>";
const char* AtmiBrokerServerXml::Max_Channels_End_Tag = (char*) "</MAX_CHANNELS>\n";

const char* AtmiBrokerServerXml::Max_Suppliers_Begin_Tag = (char*) "    <MAX_SUPPLIERS>";
const char* AtmiBrokerServerXml::Max_Suppliers_End_Tag = (char*) "</MAX_SUPPLIERS>\n";

const char* AtmiBrokerServerXml::Max_Consumers_Begin_Tag = (char*) "    <MAX_CONSUMERS>";
const char* AtmiBrokerServerXml::Max_Consumers_End_Tag = (char*) "</MAX_CONSUMERS>\n";

const char* AtmiBrokerServerXml::Orb_Type_Begin_Tag = (char*) "    <ORB_TYPE>";
const char* AtmiBrokerServerXml::Orb_Type_End_Tag = (char*) "</ORB_TYPE>\n";

const char* AtmiBrokerServerXml::Service_Names_Begin_Tag = (char*) "    <SERVICE_NAMES>\n";
const char* AtmiBrokerServerXml::Service_Names_End_Tag = (char*) "    </SERVICE_NAMES>\n";

const char* AtmiBrokerServerXml::Service_Name_Begin_Tag = (char*) "      <SERVICE_NAME>";
const char* AtmiBrokerServerXml::Service_Name_End_Tag = (char*) "</SERVICE_NAME>\n";

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static int serviceNameCount = 0;

static bool processingServer = false;
static bool processingName = false;
//TODO -NOT IMPLEMENTED static bool processingMaxReplicas		= false;
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

const char *
AtmiBrokerServerXml::writeXmlDescriptor(AtmiBroker::ServerInfo* serverData) {
	return NULL;
}

static void XMLCALL startElement
(void *userData, const char *name, const char **atts)
{
	ServerMetadata* aServerStructPtr = (ServerMetadata*)userData;

	if (strcmp(name, "SERVER xmnls") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "new server ");
		processingServer = true;

	}
	else if (strcmp(name, "NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing NAME for server ");
		processingName = true;
	}
	//TODO -NOT IMPLEMENTED   else if (strcmp(name, "MAX_REPLICAS") == 0)
	//  {
	//	  userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Replicas for server ");
	//  	  processingMaxReplicas = true;
	//  }
	else if (strcmp(name, "MAX_CHANNELS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Channels for server ");
		processingMaxChannels = true;
	}
	else if (strcmp(name, "MAX_SUPPLIERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Suppliers for server ");
		processingMaxSuppliers = true;
	}
	else if (strcmp(name, "MAX_CONSUMERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Max Consumers for server ");
		processingMaxConsumers = true;
	}
	else if (strcmp(name, "ORB_TYPE") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Orb Type for server ");
		processingOrbType = true;
	}
	else if (strcmp(name, "SERVICE_NAMES") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Service Names for server ");
		processingServiceNames = true;
	}
	else if (strcmp(name, "SERVICE_NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "processing Service Name for server ");
		processingServiceName = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement
(void *userData, const char *name)
{
	ServerMetadata* aServerStructPtr = (ServerMetadata*)userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing NAME %s", last_value);
		processingName = false;
		strcpy(server, last_value);
	}
	//TODO -NOT IMPLEMENTED   else if (strcmp(last_element, "MAX_REPLICAS") == 0)
	//  {
	//	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxReplicas %s", last_value);
	//	processingMaxReplicas		= false;
	//	aServerStructPtr->maxReplicas = (short)atol(last_value);
	//	maxReplicas = atol(last_value);
	//  }
	else if (strcmp(last_element, "MAX_CHANNELS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxChannels %s", last_value);
		processingMaxChannels = false;
		aServerStructPtr->maxChannels = (short)atol(last_value);
		serverMaxChannels = (short)atol(last_value);
	}
	else if (strcmp(last_element, "MAX_SUPPLIERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxSuppliers %s", last_value);
		processingMaxSuppliers = false;
		aServerStructPtr->maxSuppliers = (short)atol(last_value);
		serverMaxSuppliers = (short)atol(last_value);
	}
	else if (strcmp(last_element, "MAX_CONSUMERS") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing MaxConsumers %s", last_value);
		processingMaxConsumers = false;
		aServerStructPtr->maxConsumers = (short)atol(last_value);
		serverMaxConsumers = (short)atol(last_value);
	}
	else if (strcmp(last_element, "ORB_TYPE") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing OrbType %s", last_value);
		processingOrbType = false;
		aServerStructPtr->orbType = strdup(last_value);
	}
	else if (strcmp(last_element, "SERVICE_NAMES") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing ServiceNames ");
		processingServiceNames = false;
	}
	else if (strcmp(last_element, "SERVICE_NAME") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "storing ServiceName '%s' at index %d", last_value, serviceNameCount);
		processingServiceNames = false;
		serviceNameCount++;
		//	aServerStructPtr->serviceNames.length(serviceNameCount);
		aServerStructPtr->serviceNames.push_back(strdup(last_value));
	}
	depth -= 1;
}

static void XMLCALL characterData
(void *userData, const char *cdata, int len)
{
	ServerMetadata* aServerStructPtr = (ServerMetadata*)userData;

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

void AtmiBrokerServerXml::parseXmlDescriptor(ServerMetadata* aServerStructPtr, const char * aDescriptorFileName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "in parseXmlDescriptor() %s", aDescriptorFileName);

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "read file %p", aDescriptorFile);

	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(Level::getError(), loggerAtmiBrokerServerXml, (char*) "loadfile: fstat failed on %s", aDescriptorFileName);
	}
	if (s.st_size == 0) {
		userlog(Level::getError(), loggerAtmiBrokerServerXml, (char*) "loadfile: file %s is empty", aDescriptorFileName);
	}
	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "loadfile: file %s is %d long", aDescriptorFileName, s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(Level::getError(), loggerAtmiBrokerServerXml, (char*) "loadfile: Could not allocate enough memory to load file %s", aDescriptorFileName);
	}
	for (int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "loadfile: Allocated enough memory to load file %d", s.st_size);

	//char buf[1024];
	XML_Parser parser = XML_ParserCreate(NULL);
	int done;
	strcpy(element, "");
	strcpy(value, "");
	XML_SetUserData(parser, aServerStructPtr);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, characterData);
	do {
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "buf is %s", buf);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
			return;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);
	userlog(Level::getDebug(), loggerAtmiBrokerServerXml, (char*) "leaving parseXmlDescriptor() %s", aDescriptorFileName);
}

