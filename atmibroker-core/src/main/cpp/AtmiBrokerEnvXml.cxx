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

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerEnvXml(Logger::getLogger("AtmiBrokerEnvXml"));

char* notifyServiceId;
char* namingServiceId;
char* loggingServiceId;
char domain[30];
char server[30];
char* company;
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
static bool processingCompany = false;
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
	processingCompany = false;
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

//void AtmiBrokerEnvXml::writeXmlDescriptor(AtmiBroker::EnvVariableInfoSeq* aEnvironmentStructPtr, const char * aDescriptorFileName) {
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "writeXmlDescriptor() ");
//
//	char fileName[50];
//	strcpy(fileName, AtmiBrokerEnv::ENVIRONMENT_FILE);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "file name %s" << aDescriptorFileName);
//	FILE* aTempFile = fopen(fileName, "w");
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "file opened %p" << aTempFile);
//
//	// Environment
//	fputs(Environment_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Environment_Begin_Tag);
//
//	// Environment Desc
//	fputs(Environment_Desc_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Environment_Desc_Begin_Tag);
//
//	// Server
//	fputs(Server_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Server_Begin_Tag);
//	fputs(server, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << server);
//	fputs(Server_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Server_End_Tag);
//
//	// Company
//	fputs(Company_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Company_Begin_Tag);
//	fputs(company, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << company);
//	fputs(Company_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Company_End_Tag);
//
//	// Domain
//	fputs(Domain_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Domain_Begin_Tag);
//	fputs(domain, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << domain);
//	fputs(Domain_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Domain_End_Tag);
//
//	// QSpace Name
//	fputs(QSpace_Name_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << QSpace_Name_Begin_Tag);
//	fputs(queue_name, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << queue_name);
//	fputs(QSpace_Name_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << QSpace_Name_End_Tag);
//
//	// Naming Service Id
//	fputs(Naming_Service_Id_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Naming_Service_Id_Begin_Tag);
//	fputs(namingServiceId, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << namingServiceId);
//	fputs(Naming_Service_Id_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Naming_Service_Id_End_Tag);
//
//	// Notify Service Id
//	fputs(Notify_Service_Id_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Notify_Service_Id_Begin_Tag);
//	fputs(notifyServiceId, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << notifyServiceId);
//	fputs(Notify_Service_Id_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Notify_Service_Id_End_Tag);
//
//	// Logging Service Id
//	fputs(Logging_Service_Id_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Logging_Service_Id_Begin_Tag);
//	fputs(loggingServiceId, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << loggingServiceId);
//	fputs(Logging_Service_Id_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Logging_Service_Id_End_Tag);
//
//	// Transaction Factory Id
//	fputs(Trans_Factory_Id_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Trans_Factory_Id_Begin_Tag);
//	fputs(transFactoryId, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << transFactoryId);
//	fputs(Trans_Factory_Id_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Trans_Factory_Id_End_Tag);
//
//	// XA Resource Id
//	fputs(XA_Resource_Mgr_Id_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Resource_Mgr_Id_Begin_Tag);
//	fputs(xaResourceMgrId, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << xaResourceMgrId);
//	fputs(XA_Resource_Mgr_Id_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Resource_Mgr_Id_End_Tag);
//
//	// XA Resource Name
//	fputs(XA_Resource_Name_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Resource_Name_Begin_Tag);
//	fputs(xaResourceName, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << xaResourceName);
//	fputs(XA_Resource_Name_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Resource_Name_End_Tag);
//
//	// XA Open String
//	fputs(XA_Open_String_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Open_String_Begin_Tag);
//	fputs(xaOpenString, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << xaOpenString);
//	fputs(XA_Open_String_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Open_String_End_Tag);
//
//	// XA Close String
//	fputs(XA_Close_String_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Close_String_Begin_Tag);
//	fputs(xaCloseString, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << xaCloseString);
//	fputs(XA_Close_String_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Close_String_End_Tag);
//
//	// XA Thread Model
//	fputs(XA_Thread_Model_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Thread_Model_Begin_Tag);
//	if (xaThreadModel) {
//		fputs("TRUE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote TRUE");
//	} else {
//		fputs("FALSE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote FALSE");
//	}
//	fputs(XA_Thread_Model_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Thread_Model_End_Tag);
//
//	// XA Automatic Association
//	fputs(XA_Automatic_Association_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Automatic_Association_Begin_Tag);
//	if (xaAutomaticAssociation) {
//		fputs("TRUE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote TRUE");
//	} else {
//		fputs("FALSE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote FALSE");
//	}
//	fputs(XA_Automatic_Association_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Automatic_Association_End_Tag);
//
//	// XA Dynamic Registration Optimization
//	fputs(XA_Dynamic_Registration_Optimization_Begin_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Dynamic_Registration_Optimization_Begin_Tag);
//	if (xaDynamicRegistrationOptimization) {
//		fputs("TRUE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote TRUE");
//	} else {
//		fputs("FALSE", aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote FALSE");
//	}
//	fputs(XA_Dynamic_Registration_Optimization_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << XA_Dynamic_Registration_Optimization_End_Tag);
//
//	// Env Variables
//	fputs(Env_Variables_Begin_Tag, aTempFile);
//
//	for (unsigned int i = 0; i < aEnvironmentStructPtr->length(); i++) {
//		// Env Variable
//		fputs(Env_Variable_Begin_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Variable_Begin_Tag);
//
//		// Env Name
//		fputs(Env_Name_Begin_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Name_Begin_Tag);
//		fputs((const char*) (*aEnvironmentStructPtr)[i].name, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << (const char*)(*aEnvironmentStructPtr)[i].name);
//		fputs(Env_Name_End_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Name_End_Tag);
//
//		// Env Value
//		fputs(Env_Value_Begin_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Value_Begin_Tag);
//		fputs((const char*) (*aEnvironmentStructPtr)[i].value, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << (const char*)(*aEnvironmentStructPtr)[i].value);
//		fputs(Env_Value_End_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Value_End_Tag);
//
//		// Env Variable
//		fputs(Env_Variable_End_Tag, aTempFile);
//		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Variable_End_Tag);
//	}
//
//	// Env Variables
//	fputs(Env_Variables_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Env_Variables_End_Tag);
//
//	// Environment Desc
//	fputs(Environment_Desc_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Environment_Desc_End_Tag);
//
//	// Environment
//	fputs(Environment_End_Tag, aTempFile);
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "wrote '%s'" << Environment_End_Tag);
//
//	fflush(aTempFile);
//	fclose(aTempFile);
//
//	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "leaving writeXmlDescriptor() ");
//}

static void XMLCALL
startElement(void *userData, const char *name, const char **atts) {
	std::vector<envVar_t>* aEnvironmentStructPtr = (std::vector<envVar_t>*) userData;

	if (strcmp(name, "ENVIRONMENT xmnls") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "new server ");
		processingEnvironment = true;

	} else if (strcmp(name, "SERVER") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Server for environment ");
		processingServer = true;
	} else if (strcmp(name, "COMPANY") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Company for environment ");
		processingCompany = true;
	} else if (strcmp(name, "DOMAIN") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Domain for environment ");
		processingDomain = true;
	} else if (strcmp(name, "QSPACE_NAME") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing QSpaceName for environment ");
		processingQSpaceName = true;
	} else if (strcmp(name, "NAMING_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing NamingServiceId for environment ");
		processingNamingServiceId = true;
	} else if (strcmp(name, "NOTIFY_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing NotifyServiceId for environment ");
		processingNotifyServiceId = true;
	} else if (strcmp(name, "LOGGING_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing LoggingServiceId for environment ");
		processingLoggingServiceId = true;
	} else if (strcmp(name, "TRANS_FACTORY_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing TransFactoryId for environment ");
		processingTransFactoryId = true;
	} else if (strcmp(name, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaResourceMgrId for environment ");
		processingXaResourceMgrId = true;
	} else if (strcmp(name, "XA_RESOURCE_NAME") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaResourceName for environment ");
		processingXaResourceName = true;
	} else if (strcmp(name, "XA_OPEN_STRING") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaOpenString for environment ");
		processingXaOpenString = true;
	} else if (strcmp(name, "XA_CLOSE_STRING") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaCloseString for environment ");
		processingXaCloseString = true;
	} else if (strcmp(name, "XA_THREAD_MODEL") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaThreadModel for environment ");
		processingXaThreadModel = true;
	} else if (strcmp(name, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaAutomaticAssociation for environment ");
		processingXaAutomaticAssociation = true;
	} else if (strcmp(name, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing XaDynamicRegistrationOptimization for environment ");
		processingXaDynamicRegistrationOptimization = true;
	} else if (strcmp(name, "ENV_VARIABLES") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Env Variables for environment ");
		processingEnvVariables = true;
	} else if (strcmp(name, "ENV_VARIABLE") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Env Variable for environment ");
		processingEnvVariable = true;
		envVariableCount++;
		envVar_t envVar;
		(*aEnvironmentStructPtr).push_back(envVar);
	} else if (strcmp(last_element, "NAME") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Env Name for environment ");
		processingEnvName = true;
	} else if (strcmp(last_element, "VALUE") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "processing Env Value for environment ");
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

	if (strcmp(last_element, "COMPANY") == 0) {
		storedElement = true;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing COMPANY %s" << last_value);
		processingCompany = false;
		company = strdup(last_value);
	} else if (strcmp(last_element, "DOMAIN") == 0) {
		storedElement = true;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing Domain %s" << last_value);
		processingDomain = false;
		strcpy(domain, last_value);
	} else if (strcmp(last_element, "QSPACE_NAME") == 0) {
		storedElement = true;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing QSpaceName %s" << last_value);
		processingQSpaceName = false;
		queue_name = strdup(last_value);
	} else if (strcmp(last_element, "NAMING_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing NamingServiceId %s" << last_value);
		processingNamingServiceId = false;
		namingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "NOTIFY_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing NotifyServiceId %s" << last_value);
		processingNotifyServiceId = false;
		notifyServiceId = strdup(last_value);
	} else if (strcmp(last_element, "LOGGING_SERVICE_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing LoggingServiceId %s" << last_value);
		processingLoggingServiceId = false;
		loggingServiceId = strdup(last_value);
	} else if (strcmp(last_element, "TRANS_FACTORY_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing TransFactoryId %s" << last_value);
		processingTransFactoryId = false;
		transFactoryId = strdup(last_value);
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaResourceMgrId %s" << last_value);
		processingXaResourceMgrId = false;
		xaResourceMgrId = strdup(last_value);
	} else if (strcmp(last_element, "XA_RESOURCE_MGR_ID") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaResourceName %s" << last_value);
		processingXaResourceName = false;
		xaResourceName = strdup(last_value);
	} else if (strcmp(last_element, "XA_OPEN_STRING") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaOpenString %s" << last_value);
		processingXaOpenString = false;
		xaOpenString = strdup(last_value);
	} else if (strcmp(last_element, "XA_CLOSE_STRING") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaCloseString %s" << last_value);
		processingXaCloseString = false;
		xaCloseString = strdup(last_value);
	} else if (strcmp(last_element, "XA_THREAD_MODEL") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaThreadModel %s" << last_value);
		processingXaThreadModel = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaThreadModel = true;
		else
			xaThreadModel = false;
	} else if (strcmp(last_element, "XA_AUTOMATIC_ASSOCIATION") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaAutomaticAssociation %s" << last_value);
		processingXaAutomaticAssociation = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaAutomaticAssociation = true;
		else
			xaAutomaticAssociation = false;
	} else if (strcmp(last_element, "XA_DYNAMIC_REGISTRATION_OPTIMIZATION") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing XaDynamicRegistrationOptimization %s" << last_value);
		processingXaDynamicRegistrationOptimization = false;
		if (strcmp(last_value, "TRUE") == 0)
			xaDynamicRegistrationOptimization = true;
		else
			xaDynamicRegistrationOptimization = false;
	} else if (strcmp(last_element, "ENV_VARIABLES") == 0) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "stored EnvVariables ");
		processingEnvVariables = false;
	} else if (strcmp(last_element, "ENV_VARIABLE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "stored EnvVariable at index %d" << index);
		processingEnvVariable = false;
	} else if (strcmp(last_element, "NAME") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing EnvName '%s' at index %d" << last_value << index);
		processingEnvName = false;
		(*aEnvironmentStructPtr)[index].name = strdup(last_value);
	} else if (strcmp(last_element, "VALUE") == 0) {
		int index = envVariableCount - 1;
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "storing Env Value '%s' at index %d" << last_value << index);
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
	LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "value is '%s'" << value);
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
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "reading file");
		size_t len = fread(buf, 1, s.st_size, aDescriptorFile);
		LOG4CXX_DEBUG(loggerAtmiBrokerEnvXml, (char*) "length is '%d'" << len);
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

