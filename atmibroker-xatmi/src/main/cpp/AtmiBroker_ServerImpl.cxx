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

//
// Servant which implements the AtmiBroker::Server interface.
//

#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#include <orbsvcs/CosNamingS.h>
#elif ORBIX_COMP
#include <omg/orb.hh>
#include <omg/CosNaming.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#include <CosNaming_c.hh>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "AtmiBroker_ServerImpl.h"
#include "AtmiBroker.h"

#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerServiceFacMgr.h"
#include "AtmiBrokerServiceFac.h"
#include "AtmiBrokerServerXml.h"
#include "AtmiBrokerEnv.h"

//TODO NEEDS extern "C" __declspec(dllexport) - unacceptable
//#include "CallDynamicSymbols.h"

#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ServerImpl(Logger::getLogger("AtmiBroker_ServerImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::Server*
AtmiBroker_ServerImpl::_create(PortableServer::POA_ptr the_poa) {
	return new AtmiBroker_ServerImpl(the_poa);
}

// AtmiBroker_ServerImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ServerImpl::AtmiBroker_ServerImpl(PortableServer::POA_ptr the_poa) :
	IT_ServantBaseOverrides(the_poa) {
	// Intentionally empty.
	nextClientId = 0L;

	std::string serverFileName = "SERVER.xml";
	AtmiBrokerServerXml aAtmiBrokerServerXml;
	aAtmiBrokerServerXml.parseXmlDescriptor(&serverInfo, serverFileName.c_str());
	serverName = server;
}

// ~AtmiBroker_ServerImpl destructor.
//
AtmiBroker_ServerImpl::~AtmiBroker_ServerImpl() {
	// Intentionally empty.
	//
}

int AtmiBroker_ServerImpl::serverinit(int argc, char *argv[]) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "serverinit");
	server_init();
	return (0);
}

void AtmiBroker_ServerImpl::serverdone() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "serverdone");
	server_done();
}

// server_init() -- Implements IDL operation "AtmiBroker::Server::server_init".
//
CORBA::Short AtmiBroker_ServerImpl::server_init() throw (CORBA::SystemException ) {
	int toReturn = 1;
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_init(): called.");

	/* TODO FUNCTION NEEDS extern "C" __declspec(dllexport)
	 for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
	 char * serviceName = (char*)serverInfo.serviceNames[i].c_str();
	 void (*function)(TPSVCINFO*) = getFunction("C:/Projects/blacktie/product/atmibroker/target/atmibroker-tests_vc9/atmibroker-tests_vc9.exe", serviceName);
	 if (function != NULL) {
	 int advertised = tpadvertise(serviceName, function);
	 if (advertised < 0) {
	 toReturn = -1;
	 }
	 }
	 }
	 */

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_init(): finished.");

	return toReturn;
}

// server_done() -- Implements IDL operation "AtmiBroker::Server::server_done".
//
void AtmiBroker_ServerImpl::server_done() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_done()");

	// unadvertise(); // DO NOT DO THIS SINCE THE NamingService needs the Persistant IOR for re-launching!!!

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		char* svcname = (char*) serverInfo.serviceNames[i].c_str();
		if (isAdvertised(svcname)) {
			unadvertiseService(svcname);
		}
	}
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_done(): returning.");
}

char *
AtmiBroker_ServerImpl::getServerName() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServerName ");
	return serverName;
}

void AtmiBroker_ServerImpl::unadvertise() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "unadvertise %s", serverName);

	CosNaming::Name * name;

	name = server_default_context->to_name(serverName);
	server_name_context->unbind(*name);

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "unadvertised %s", serverName);
}

void AtmiBroker_ServerImpl::advertisedService(char * serviceName) {
	advertisedServices.push_back(serviceName);
}

bool AtmiBroker_ServerImpl::advertiseService(char * serviceName, void(*func)(TPSVCINFO *)) {
	if (!serviceName || strcmp(serviceName, "") == 0) {
		tperrno = TPEINVAL;
		return false;
	}

	bool found = false;
	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		if (strncmp(serverInfo.serviceNames[i].c_str(), serviceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			found = true;
		}
	}
	if (!found) {
		tperrno = TPELIMIT;
		return false;
	}
	void (*serviceFunction)(TPSVCINFO*) = AtmiBrokerServiceFacMgr::get_instance()->getServiceMethod(serviceName);
	if (serviceFunction != NULL) {
		if (serviceFunction == func) {
			return true;
		} else {
			tperrno = TPEMATCH;
			return false;
		}
	}

	bool toReturn = false;

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "advertiseService(): '%s'", serviceName);

	AtmiBroker_ServiceFactoryImpl *tmp_factory_servant = NULL;
	AtmiBroker_ServiceManagerImpl *tmp_manager_servant = NULL;

	// create Poa for Service Factory
	PortableServer::POA_var aFactoryPoaPtr = create_service_factory_poa(serviceName);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "advertiseService():  created service factory poa");
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "advertiseService():  aFactoryPoaPtr %p", (void*) aFactoryPoaPtr);

	// create Poa for Service Manager
	PortableServer::POA_var aManagerPoaPtr = create_service_manager_poa(serviceName);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "advertiseService():  aManagerPoaPtr %p", (void*) aManagerPoaPtr);

	// TODO MAYBE CAN ONLY ADVERTISE PRE-KNOWN?
	if (true) {
		// TODO TEST_SERVICE
		// TOLOWER?
		char servicePoaName[80];
		strcpy(servicePoaName, serviceName);
		strcat(servicePoaName, "_service_poa");

		// TOLOWER?
		char serviceConfigFilename[80];
		strcpy(serviceConfigFilename, serviceName);
		strcat(serviceConfigFilename, ".xml");

		tmp_factory_servant = new AtmiBroker_ServiceFactoryImpl(aFactoryPoaPtr, serviceName, servicePoaName, serviceConfigFilename);
		userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) " tmp_factory_servant %p", (void*) tmp_factory_servant);
		tmp_manager_servant = new AtmiBroker_ServiceManagerImpl(aManagerPoaPtr, serviceName);
		userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) " tmp_manager_servant %p", (void*) tmp_manager_servant);

		// create reference for Service Factory and cache
		create_service_factory(tmp_factory_servant, aFactoryPoaPtr, serviceName, func);
		// create reference for Service Manager
		create_service_manager(tmp_manager_servant, aManagerPoaPtr, serviceName);
		advertisedService(serviceName);
		toReturn = true;
	}
	return toReturn;
}

// TODO CLEANUP SERVICE POA AND SERVANT CACHE
void AtmiBroker_ServerImpl::unadvertiseService(char * serviceName) {
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			userlog(Level::getInfo(), loggerAtmiBroker_ServerImpl, (char*) "unadvertiseService(): %s", serviceName);

			remove_service_factory(serviceName);
			remove_service_manager(serviceName);
			advertisedServices.erase(i);
			break;
		}
	}
}

bool AtmiBroker_ServerImpl::isAdvertised(char * serviceName) {
	bool toReturn = false;
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			toReturn = true;
		}
	}
	return toReturn;
}

CORBA::Long AtmiBroker_ServerImpl::register_client(const AtmiBroker::ClientInfo& client_info) throw (CORBA::SystemException) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "register_client(): %s", (const char*) ((AtmiBroker::ClientInfo&) client_info).callback_ior);

	nextClientId++;

	AtmiBroker::ClientInfo_var clientInfo = new AtmiBroker::ClientInfo(client_info);
	clientInfo->callback_ior = CORBA::string_dup(client_info.callback_ior);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "register_client(): after creating copy : %s", (const char*) clientInfo->callback_ior);
	clientInfo->client_id = nextClientId;

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "register_client(): adding clientInfo: %p to vector", (void*) clientInfo);
	clientInfoVector.push_back(clientInfo);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "register_client(): added clientInfo: %p to vector", (void*) clientInfo);

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "returning nextClientId: %d", nextClientId);

	return nextClientId;
}

CORBA::Boolean AtmiBroker_ServerImpl::deregister_client(const AtmiBroker::ClientInfo& client_info) throw (CORBA::SystemException) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "deregister_client(): %p", (void*) &client_info);

	CORBA::Boolean _result = false;

	for (std::vector<AtmiBroker::ClientInfo_var>::iterator it = clientInfoVector.begin(); it != clientInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "deregister_client: next ior is: %s", (const char*) (*it)->callback_ior);
		if (strcmp((*it)->callback_ior, client_info.callback_ior) == 0) {
			userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "deregister_client: found matching ior ");
			clientInfoVector.erase(it);
			userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "deregister_client: removed match ");
			return true;
		}
	}

	return _result;
}

char*
AtmiBroker_ServerImpl::get_client_callback(const AtmiBroker::ClientInfo& client_info) throw (CORBA::SystemException) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_callback() for client_id: %d", client_info.client_id);

	char* _result = (char*) "";

	for (std::vector<AtmiBroker::ClientInfo_var>::iterator it = clientInfoVector.begin(); it != clientInfoVector.end(); it++) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_callback() next id is: %d", (char*) (*it)->client_id);
		if ((*it)->client_id == client_info.client_id) {
			const char * aResult = (const char *) (*it)->callback_ior;
			userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_callback() found match client id return ior %s", aResult);
			return (char*) aResult;
		}
	}

	return _result;
}

// get_queue_log() -- Implements IDL operation "AtmiBroker::Server::get_queue_log".
//
CORBA::Long AtmiBroker_ServerImpl::get_queue_log(const char* queue_name) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_queue_log() for queue: %s", queue_name);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_queue_log() server queue_name is %s", AtmiBrokerEnv::get_instance()->getenv((char*) "QSPACE_NAME"));
	if ((strcmp(AtmiBrokerEnv::get_instance()->getenv((char*) "QSPACE_NAME"), queue_name) == 0) || (strcmp((char*) "", queue_name) == 0)) {
		//TODO READD userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_queue_log() return get_queue_log %d", AtmiBrokerLog::get_instance()->getQueueLogId());
		//TODO READD return AtmiBrokerLog::get_instance()->getQueueLogId();
		return -1;
	} else
		return -1;
}

// get_server_info() -- Implements IDL operation "AtmiBroker::Server::get_server_info".
//
AtmiBroker::ServerInfo*
AtmiBroker_ServerImpl::get_server_info() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_server_info()");

	AtmiBroker::ServerInfo_var aServerInfo = new AtmiBroker::ServerInfo();

	aServerInfo->maxChannels = serverInfo.maxChannels;
	aServerInfo->maxSuppliers = serverInfo.maxSuppliers;
	aServerInfo->maxConsumers = serverInfo.maxConsumers;
	aServerInfo->logLevel = serverInfo.logLevel;
	aServerInfo->securityType = CORBA::string_dup(serverInfo.securityType.c_str());
	aServerInfo->orbType = CORBA::string_dup(serverInfo.orbType.c_str());
	aServerInfo->queueSpaceName = CORBA::string_dup(queue_name);

	//aServerInfo->serviceNames.length(serverInfo.serviceNames.size());
	std::queue<AtmiBroker::octetSeq *> returnData;
	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		std::string serviceName = serverInfo.serviceNames[i];
		//AtmiBroker::octetSeq * aOctetSeq =
		returnData.push(new AtmiBroker::octetSeq(serviceName.length(), serviceName.length(), (unsigned char *) serviceName.c_str(), true));//*aOctetSeq));
	}
	AtmiBroker::ServiceNameSeq_var aOctetSeq = new AtmiBroker::ServiceNameSeq(serverInfo.serviceNames.size(), serverInfo.serviceNames.size(), returnData.front(), true);
	aServerInfo->serviceNames = aOctetSeq;

	return aServerInfo._retn();
}

// get_all_service_info() -- Implements IDL operation "AtmiBroker::Server::get_all_service_info".
//
AtmiBroker::ServiceInfoSeq*
AtmiBroker_ServerImpl::get_all_service_info() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_all_service_info()");

	AtmiBroker::ServiceInfoSeq_var aServiceInfoSeq = new AtmiBroker::ServiceInfoSeq();
	aServiceInfoSeq->length(serverInfo.serviceNames.size());

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		AtmiBroker::ServiceInfo * aServiceInfoPtr = AtmiBrokerServiceFacMgr::get_instance()->getServiceFactory((char*) serverInfo.serviceNames[i].c_str())->get_service_info();
		(*aServiceInfoSeq)[i] = *aServiceInfoPtr;
	}
	return aServiceInfoSeq._retn();
}

// get_service_info() -- Implements IDL operation "AtmiBroker::Server::get_service_info".
//
AtmiBroker::ServiceInfo*
AtmiBroker_ServerImpl::get_service_info(const char* service_name) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_service_info() %s", service_name);

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		if (strcmp(service_name, (char*) serverInfo.serviceNames[i].c_str()) == 0) {
			return AtmiBrokerServiceFacMgr::get_instance()->getServiceFactory(service_name)->get_service_info();
		}
	}
	userlog(Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "get_service_info() Service %s NOT FOUND", service_name);
	return (AtmiBroker::ServiceInfo*) NULL;

}

// get_environment_variable_info() -- Implements IDL operation "AtmiBroker::Server::get_environment_variable_info".
//
AtmiBroker::EnvVariableInfoSeq*
AtmiBroker_ServerImpl::get_environment_variable_info() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info()");

	std::vector<envVar_t> & aEnvVarInfoSeq = AtmiBrokerEnv::get_instance()->getEnvVariableInfoSeq();

	AtmiBroker::EnvVariableInfoSeq* aEnvVariableInfoSeqPtr = new AtmiBroker::EnvVariableInfoSeq();

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() setting length to %d", aEnvVarInfoSeq.size());
	aEnvVariableInfoSeqPtr->length(aEnvVarInfoSeq.size());
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() set length to %d", aEnvVariableInfoSeqPtr->length());
	int j = 0;
	for (std::vector<envVar_t>::iterator i = aEnvVarInfoSeq.begin(); i != aEnvVarInfoSeq.end(); i++) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() env name %s ", (*i).name);
		(*aEnvVariableInfoSeqPtr)[j].name = strdup((*i).name);
		(*aEnvVariableInfoSeqPtr)[j].value = strdup((*i).value);
		j++;
	}
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() returning ");
	return aEnvVariableInfoSeqPtr;
}

// get_client_info() -- Implements IDL operation "AtmiBroker::Server::get_client_info".
//
AtmiBroker::ClientInfoSeq*
AtmiBroker_ServerImpl::get_client_info() throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_info()");

	AtmiBroker::ClientInfoSeq* aClientInfoSeqPtr = new AtmiBroker::ClientInfoSeq();

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_info() setting length to %d", clientInfoVector.size());
	aClientInfoSeqPtr->length(clientInfoVector.size());
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_info() set length to %d", aClientInfoSeqPtr->length());

	int i = 0;
	for (std::vector<AtmiBroker::ClientInfo_var>::iterator it = clientInfoVector.begin(); it != clientInfoVector.end(); it++) {
		i++;
		(*aClientInfoSeqPtr)[i].maxChannels = (*it)->maxChannels;
		(*aClientInfoSeqPtr)[i].maxSuppliers = (*it)->maxSuppliers;
		(*aClientInfoSeqPtr)[i].maxConsumers = (*it)->maxConsumers;
		(*aClientInfoSeqPtr)[i].logLevel = (*it)->logLevel;
	}
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_client_info() returning ");

	return aClientInfoSeqPtr;
}

// set_server_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_server_descriptor".
//
void AtmiBroker_ServerImpl::set_server_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() %s", xml_descriptor);

	std::string serverFileName = "server.xml";

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file name %s", serverFileName.c_str());
	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file opened %p", aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file written and closed ");
}

// set_service_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_service_descriptor".
//
void AtmiBroker_ServerImpl::set_service_descriptor(const char* service_name, const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor() %s %s", service_name, xml_descriptor);

	std::string serverFileName = "server.xml";

	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file %s opened %p", serverFileName.c_str(), aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file written and closed ");
}

// set_environment_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_environment_descriptor".
//
void AtmiBroker_ServerImpl::set_environment_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() %s", xml_descriptor);

	FILE* aTempFile = fopen("Environment.xml", "w");
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file Environment.xml opened %p", aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file written and closed ");
}

// stop_service() -- Implements IDL operation "AtmiBroker::Server::stop_service".
//
void AtmiBroker_ServerImpl::stop_service(const char* service_name) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() %s", service_name);

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() next service %s", (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() found matching service %s...calling tpunadvertise", service_name);
#ifndef VBC_COMP
				tpunadvertise((char*) serverInfo.serviceNames[i].c_str());
#else
				tpunadvertise((char*)serverInfo.serviceNames[i]);
#endif
				userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() called tpunadvertise");
				return;
			}
		}
		userlog(Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() DID NOT find matching service %s...", service_name);
	} catch (CORBA::Exception &e) {
		userlog(Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() Could not stop service - exception: %s...", e._name());
	}
}

// start_service() -- Implements IDL operation "AtmiBroker::Server::start_service".
//
void AtmiBroker_ServerImpl::start_service(const char* service_name) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service() %s", service_name);

	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopping service first ");
	stop_service(service_name);
	userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopped service");

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  next service %s", (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  found matching service %s...calling tpadvertise", service_name);
#ifndef VBC_COMP
				tpadvertise((char*) serverInfo.serviceNames[i].c_str(), NULL);
#else
				tpadvertise((char*)serverInfo.serviceNames[i], NULL);
#endif
				userlog(Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  called tpadvertise");
				return;
			}
		}
		userlog(Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  DID NOT find matching service %s...", service_name);
	} catch (CORBA::Exception &e) {
		userlog(Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  Could not start service - exception: %s...", e._name());
	}
}

