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

#include "AtmiBrokerServiceXml.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServiceXml(Logger::getLogger("AtmiBrokerServiceXml"));

const char* AtmiBrokerServiceXml::Service_Begin_Tag = (char*) "<?xml version=\"1.0\"?>\n<SERVICE xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";

const char* AtmiBrokerServiceXml::Service_End_Tag = (char*) "</SERVICE>\n";

const char* AtmiBrokerServiceXml::Service_Desc_Begin_Tag = (char*) "  <SERVICE_DESCRIPTION>";
const char* AtmiBrokerServiceXml::Service_Desc_End_Tag = (char*) "  </SERVICE_DESCRIPTION>\n";

const char* AtmiBrokerServiceXml::Max_Cache_Begin_Tag = (char*) "    <MAX>";
const char* AtmiBrokerServiceXml::Max_Cache_End_Tag = (char*) "</MAX>\n";

const char* AtmiBrokerServiceXml::Min_Cache_Begin_Tag = (char*) "    <MIN>";
const char* AtmiBrokerServiceXml::Min_Cache_End_Tag = (char*) "</MIN>\n";

const char* AtmiBrokerServiceXml::Min_Available_Cache_Begin_Tag = (char*) "    <AVAILABLE>";
const char* AtmiBrokerServiceXml::Min_Available_Cache_End_Tag = (char*) "</AVAILABLE>\n";

static char last_element[50];
static char last_value[50];

static char element[50];
static char value[50];

static int depth = 0;

static int indexTraitArray = 0;
static int processingTraitArray = false;

static bool processingService = false;
static bool processingMaxCache = false;
static bool processingMinCache = false;
static bool processingMinAvailableCache = false;

AtmiBrokerServiceXml::AtmiBrokerServiceXml() {
}

AtmiBrokerServiceXml::~AtmiBrokerServiceXml() {
}

const char *
AtmiBrokerServiceXml::writeXmlDescriptor(AtmiBroker::ServiceInfo* serviceData) {
	return NULL;
}

static void XMLCALL startElement
(void *userData, const char *name, const char **atts)
{
	AtmiBroker::ServiceInfo* aServiceStructPtr = (AtmiBroker::ServiceInfo*)userData;

	if (strcmp(name, "SERVICE ") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "new service ");
		processingService = true;
	}
	else if (strcmp(name, "MAX") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "processing MAX Cache for service ");
		processingMaxCache = true;
	}
	else if (strcmp(name, "MIN") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "processing MIN Cache for service ");
		processingMinCache = true;
	}
	else if (strcmp(name, "AVAILABLE") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "processing Min AVAILABLE for service ");
		processingMinAvailableCache = true;
	}
	strcpy(element, name);
	strcpy(value, "");

	depth += 1;
}

static void XMLCALL endElement
(void *userData, const char *name)
{
	AtmiBroker::ServiceInfo* aServiceStructPtr = (AtmiBroker::ServiceInfo*)userData;

	strcpy(last_element, name);
	strcpy(last_value, value);

	if (strcmp(last_element, "MAX") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "storing MaxCache %s", last_value);
		processingMaxCache = false;
		aServiceStructPtr->maxSize = (short)atol(last_value);
	}
	else if (strcmp(last_element, "MIN") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "storing MinCache %s", last_value);
		processingMinCache = false;
		aServiceStructPtr->minSize = (short)atol(last_value);
	}
	else if (strcmp(last_element, "AVAILABLE") == 0)
	{
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) " storing MinAvailable %s", last_value);
		processingMinAvailableCache = false;
		aServiceStructPtr->minAvailableSize = (short)atol(last_value);
	}
	depth -= 1;
}

static void XMLCALL characterData
(void *userData, const char *cdata, int len)
{
	AtmiBroker::ServiceInfo* aServiceStructPtr = (AtmiBroker::ServiceInfo*)userData;

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

void AtmiBrokerServiceXml::parseXmlDescriptor(AtmiBroker::ServiceInfo* aServiceStructPtr, const char * aDescriptorFileName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "in parseXmlDescriptor() %s", aDescriptorFileName);

	struct stat s; /* file stats */
	FILE *aDescriptorFile = fopen(aDescriptorFileName, "r");

	userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "read file %p", aDescriptorFile);

	if (!aDescriptorFile) {
		userlog(Level::getWarn(), loggerAtmiBrokerServiceXml, (char*) "parseXmlDescriptor could not load service config %s", aDescriptorFileName);
		return;
	}
	/* Use fstat to obtain the file size */
	if (fstat(fileno(aDescriptorFile), &s) != 0) {
		/* fstat failed */
		userlog(Level::getError(), loggerAtmiBrokerServiceXml, (char*) "loadfile: fstat failed on %s", aDescriptorFileName);
	}
	if (s.st_size == 0) {
		userlog(Level::getError(), loggerAtmiBrokerServiceXml, (char*) "loadfile: file %s is empty", aDescriptorFileName);
	}
	userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "loadfile: file %s is %d long", aDescriptorFileName, s.st_size);

	char *buf = (char *) malloc(sizeof(char) * s.st_size);
	if (!buf) {
		/* malloc failed */
		userlog(Level::getError(), loggerAtmiBrokerServiceXml, (char*) "loadfile: Could not allocate enough memory to load file %s", aDescriptorFileName);
	}
	for (int i = 0; i < sizeof(buf); i++)
		*(buf + i) = '\0';
	//memcpy(buf,'\0',s.st_size);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "loadfile: Allocated enough memory to load file %d", s.st_size);

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
		userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "buf is %s", buf);
		done = len < sizeof(buf);
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "%d at line %d", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
			return;
		}
	} while (!done);
	free(buf);
	XML_ParserFree(parser);

	fflush(aDescriptorFile);
	fclose(aDescriptorFile);

	userlog(Level::getDebug(), loggerAtmiBrokerServiceXml, (char*) "leaving parseXmlDescriptor() %s", aDescriptorFileName);
}

