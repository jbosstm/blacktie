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
#include <orbsvcs/CosNamingS.h>
#endif

#include <string>
#include <queue>
#include "AtmiBrokerServer.h"
#include "AtmiBroker.h"
#include "AtmiBrokerPoaFac.h"
#include "AtmiBrokerEnv.h"
#include "log4cxx/logger.h"
#include "EndpointQueue.h"
#include "OrbManagement.h"
#include "AtmiBroker.h"
#include "userlog.h"
#include "Connection.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerPoaFac.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger("AtmiBrokerServer"));
AtmiBrokerServer * ptrServer = NULL;
CONNECTION* serverConnection;
bool serverInitialized = false;
PortableServer::POA_var server_poa;

void server_sigint_handler_callback(int sig_type) {
	userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "server_sigint_handler_callback Received shutdown signal: %d", sig_type);
	serverdone();
	abort();
}

int serverrun() {
	int toReturn = 0;
	try {
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, "Server waiting for requests...");
		((CORBA::ORB_ptr) serverConnection->orbRef)->run();
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, "Unexpected CORBA exception: %s", e._name());
		toReturn = -1;
	}
	return toReturn;
}

int serverinit() {
	_tperrno = 0;
	int toReturn = 0;
	if (!loggerInitialized) {
		if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
			log4cxx::PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
		} else {
			log4cxx::BasicConfigurator::configure();
		}
		loggerInitialized = true;
	}

	if (ptrServer == NULL) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit called");
		signal(SIGINT, server_sigint_handler_callback);
		ptrServer = new AtmiBrokerServer();
		if (!serverInitialized) {
			::serverdone();
			toReturn = -1;
		} else {
			userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "Server Running");
		}
	}
	return toReturn;
}

int serverdone() {
	_tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone called ");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone shutting down services ");
	if (ptrServer) {
		delete ptrServer;
		ptrServer = NULL;
	}
	return 0;
}

// AtmiBrokerServer constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBrokerServer::AtmiBrokerServer() {
	try {
		serverConnection = AtmiBrokerOTS::init_orb((char*) "server");
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "creating POAs for %s", server);
		AtmiBrokerPoaFac* serverPoaFactory = (AtmiBrokerPoaFac*) serverConnection->poaFactory;
		this->poa = serverPoaFactory->createServerPoa(((CORBA::ORB_ptr) serverConnection->orbRef), server, ((PortableServer::POA_ptr) serverConnection->root_poa), ((PortableServer::POAManager_ptr) serverConnection->root_poa_manager));

		AtmiBrokerServerXml aAtmiBrokerServerXml;
		aAtmiBrokerServerXml.parseXmlDescriptor(&serverInfo, (char*) "SERVER.xml");
		serverName = server;

		PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(server);
		poa->activate_object_with_id(oid, this);
		CORBA::Object_var tmp_ref = poa->create_reference_with_id(oid, "IDL:AtmiBroker/Server:1.0");

		CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) serverConnection->default_ctx)->to_name(serverName);
		((CosNaming::NamingContext_ptr) serverConnection->name_ctx)->bind(*name, tmp_ref);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_init(): finished.");

		serverInitialized = true;
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, (char*) "serverinit - Unexpected CORBA exception: %s", e._name());
		tperrno = TPESYSTEM;
	}
}

// ~AtmiBrokerServer destructor.
//
AtmiBrokerServer::~AtmiBrokerServer() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destructor ");
	server_done();

	serviceData.clear();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleted service array ");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleting services");
	AtmiBrokerMem::discard_instance();
	//TODO READD AtmiBrokerNotify::discard_instance();
	AtmiBrokerOTS::discard_instance();
	AtmiBrokerEnv::discard_instance();
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleted services");

	if (serverConnection) {
		shutdownBindings(serverConnection);
		serverConnection = NULL;
	}
	serverInitialized = false;

}

// server_init() -- Implements IDL operation "AtmiBroker::Server::server_init".
//
CORBA::Short AtmiBrokerServer::server_init() throw (CORBA::SystemException ) {
	return 0;
}

// server_done() -- Implements IDL operation "AtmiBroker::Server::server_done".
//
void AtmiBrokerServer::server_done() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_done()");

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "unadvertise " << serverName);
	if (serverConnection) {
		CosNaming::Name* name = ((CosNaming::NamingContextExt_ptr) serverConnection->default_ctx)->to_name(serverName);
		((CosNaming::NamingContext_ptr) serverConnection->name_ctx)->unbind(*name);

		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "unadvertised " << serverName);

		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			char* svcname = (char*) serverInfo.serviceNames[i].c_str();
			if (isAdvertised(svcname)) {
				unadvertiseService(svcname);
			}
		}
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_done(): returning.");
}

char *
AtmiBrokerServer::getServerName() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServerName ");
	return serverName;
}

bool AtmiBrokerServer::advertiseService(char * serviceName, void(*func)(TPSVCINFO *)) {
	if (!serviceName || strlen(serviceName) == 0) {
		tperrno = TPEINVAL;
		return false;
	}

	bool found = false;
	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		if (strncmp(serverInfo.serviceNames[i].c_str(), serviceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			found = true;
			break;
		}
	}
	if (!found) {
		tperrno = TPELIMIT;
		return false;
	}
	void (*serviceFunction)(TPSVCINFO*) = getServiceMethod(serviceName);
	if (serviceFunction != NULL) {
		if (serviceFunction == func) {
			return true;
		} else {
			tperrno = TPEMATCH;
			return false;
		}
	}

	bool toReturn = false;

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "advertiseService(): " << serviceName);

	// create reference for Service Queue and cache
	try {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "create_service_queue: " << serviceName);

		// create Poa for Service Queue
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "create_service_queue_poa: " << serviceName);
		AtmiBrokerPoaFac* poaFactory = ((AtmiBrokerPoaFac*) serverConnection->poaFactory);
		PortableServer::POA_ptr aFactoryPoaPtr = poaFactory->createServicePoa((CORBA::ORB_ptr) serverConnection->orbRef, serviceName, poa, (PortableServer::POAManager_ptr) serverConnection->root_poa_manager);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "created create_service_factory_poa: " << serviceName);

		Destination* destination = new EndpointQueue(serverConnection, aFactoryPoaPtr, serviceName);
		ServiceDispatcherPool *tmp_factory_servant = new ServiceDispatcherPool(serverConnection, destination, serviceName, func);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) " tmp_factory_servant " << tmp_factory_servant);

		addServiceDispatcherPool(serviceName, tmp_factory_servant, func);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "created ServiceDispatcherPool " << serviceName);

	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "service has already been advertised, however it appears to be by a different server (possibly with the same name), which is strange... " << serviceName);
		tperrno = TPEMATCH;
		return false;
	}

	advertisedServices.push_back(serviceName);
	LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "advertised service " << serviceName);
	toReturn = true;
	return toReturn;
}

// TODO CLEANUP SERVICE POA AND SERVANT CACHE
void AtmiBrokerServer::unadvertiseService(char * serviceName) {
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "remove_service_queue: " << serviceName);
			CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) serverConnection->default_ctx)->to_name(serviceName);
			((CosNaming::NamingContext_ptr) serverConnection->name_ctx)->unbind(*name);

			ServiceDispatcherPool* toDelete = removeServiceDispatcherPool(serviceName);
			EndpointQueue* queue = dynamic_cast<EndpointQueue*> (toDelete->getDestination());
			PortableServer::POA_ptr poa = (PortableServer::POA_ptr) queue->getPoa();
			delete toDelete;
			poa->destroy(true, true);
			poa = NULL;
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "service Queue removed" << serviceName);
			advertisedServices.erase(i);
			LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "unadvertised service " << serviceName);
			break;
		}
	}
}

bool AtmiBrokerServer::isAdvertised(char * serviceName) {
	bool toReturn = false;
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			toReturn = true;
		}
	}
	return toReturn;
}

// get_server_info() -- Implements IDL operation "AtmiBroker::Server::get_server_info".
//
AtmiBroker::ServerInfo*
AtmiBrokerServer::get_server_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_server_info()");

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
AtmiBrokerServer::get_all_service_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_all_service_info()");

	AtmiBroker::ServiceInfoSeq_var aServiceInfoSeq = new AtmiBroker::ServiceInfoSeq();
	aServiceInfoSeq->length(serverInfo.serviceNames.size());

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		SVCINFO svcInfo = getServiceDispatcherPool((char*) serverInfo.serviceNames[i].c_str())->get_service_info();
		AtmiBroker::ServiceInfo_var aServiceInfo = new AtmiBroker::ServiceInfo();
		aServiceInfo->serviceName = svcInfo.serviceName;
		aServiceInfo->poolSize = svcInfo.poolSize;
		aServiceInfo->securityType = svcInfo.securityType;
		// TODO(*aServiceInfoSeq)[i] = aServiceInfo._retn();
	}
	return aServiceInfoSeq._retn();
}

// get_environment_variable_info() -- Implements IDL operation "AtmiBroker::Server::get_environment_variable_info".
//
AtmiBroker::EnvVariableInfoSeq*
AtmiBrokerServer::get_environment_variable_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_environment_variable_info()");

	std::vector<envVar_t> & aEnvVarInfoSeq = AtmiBrokerEnv::get_instance()->getEnvVariableInfoSeq();

	AtmiBroker::EnvVariableInfoSeq* aEnvVariableInfoSeqPtr = new AtmiBroker::EnvVariableInfoSeq();

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_environment_variable_info() setting length to " << aEnvVarInfoSeq.size());
	aEnvVariableInfoSeqPtr->length(aEnvVarInfoSeq.size());
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_environment_variable_info() set length to " << aEnvVariableInfoSeqPtr->length());
	int j = 0;
	for (std::vector<envVar_t>::iterator i = aEnvVarInfoSeq.begin(); i != aEnvVarInfoSeq.end(); i++) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_environment_variable_info() env name " << (*i).name);
		(*aEnvVariableInfoSeqPtr)[j].name = strdup((*i).name);
		(*aEnvVariableInfoSeqPtr)[j].value = strdup((*i).value);
		j++;
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_environment_variable_info() returning ");
	return aEnvVariableInfoSeqPtr;
}

// set_server_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_server_descriptor".
//
void AtmiBrokerServer::set_server_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_server_descriptor() " << xml_descriptor);

	std::string serverFileName = "server.xml";

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_server_descriptor() file name " << serverFileName.c_str());
	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_server_descriptor() file opened " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_server_descriptor() file written and closed ");
}

// set_service_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_service_descriptor".
//
void AtmiBrokerServer::set_service_descriptor(const char* service_name, const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_service_descriptor() %s " << service_name << " " << xml_descriptor);

	std::string serverFileName = "server.xml";

	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_service_descriptor()  file opened " << " " << serverFileName.c_str() << " " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_service_descriptor()  file written and closed ");
}

// set_environment_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_environment_descriptor".
//
void AtmiBrokerServer::set_environment_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_environment_descriptor() " << xml_descriptor);

	FILE* aTempFile = fopen("Environment.xml", "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_environment_descriptor() file Environment.xml opened " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_environment_descriptor() file written and closed ");
}

// stop_service() -- Implements IDL operation "AtmiBroker::Server::stop_service".
//
void AtmiBrokerServer::stop_service(const char* service_name) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "stop_service() " << service_name);

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "stop_service() next service " << (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "stop_service() found matching service calling tpunadvertise" << service_name);
#ifndef VBC_COMP
				tpunadvertise((char*) serverInfo.serviceNames[i].c_str());
#else
				tpunadvertise((char*)serverInfo.serviceNames[i]);
#endif
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "stop_service() called tpunadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "stop_service() DID NOT find matching service " << service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "stop_service() Could not stop service - exception: " << e._name());
	}
}

// start_service() -- Implements IDL operation "AtmiBroker::Server::start_service".
//
void AtmiBrokerServer::start_service(const char* service_name) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service() " << service_name);

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service()  stopping service first ");
	stop_service(service_name);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service()  stopped service");

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service()  next service " << (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service()  found matching service calling tpadvertise" << service_name);
#ifndef VBC_COMP
				tpadvertise((char*) serverInfo.serviceNames[i].c_str(), NULL);
#else
				tpadvertise((char*)serverInfo.serviceNames[i], NULL);
#endif
				LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service()  called tpadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "start_service()  DID NOT find matching service " << service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "start_service()  Could not start service - exception: " << e._name());
	}
}

ServiceDispatcherPool* AtmiBrokerServer::getServiceDispatcherPool(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceDispatcherPool: " << aServiceName);

	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "found: " << (char*) (*i).serviceName);
			return (*i).serviceQueue;
		}
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceDispatcherPool out: " << aServiceName);
	return NULL;
}

void AtmiBrokerServer::addServiceDispatcherPool(char*& aServiceName, ServiceDispatcherPool*& aFactoryPtr, void(*func)(TPSVCINFO *)) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "addServiceDispatcherPool: " << aServiceName);

	ServiceData entry;
	entry.serviceName = aServiceName;
	entry.serviceQueue = aFactoryPtr;
	entry.func = func;
	serviceData.push_back(entry);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "added: " << (char*) aServiceName);
}

ServiceDispatcherPool* AtmiBrokerServer::removeServiceDispatcherPool(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removeServiceDispatcherPool: " << aServiceName);
	ServiceDispatcherPool* toReturn = NULL;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).serviceQueue;
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removing service " << (char*) (*i).serviceName);
			serviceData.erase(i);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removed: " << aServiceName);
			break;
		}
	}
	return toReturn;
}

void (*AtmiBrokerServer::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceDispatcherPool: " << aServiceName);

			for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
				if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "found: " << (char*) (*i).serviceName);
					return (*i).func;
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceMethod out: " << aServiceName);
			return NULL;
		}
