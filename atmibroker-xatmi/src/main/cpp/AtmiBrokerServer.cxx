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

//
// Servant which implements the AtmiBroker::Server interface.
//

#include <string>
#include <queue>

#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include "log4cxx/logger.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerPoaFac.h"
#include "AtmiBrokerEnv.h"
#include "userlog.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerOTS.h"
#include "OrbManagement.h"
#include "SymbolLoader.h"

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger(
		"AtmiBrokerServer"));
AtmiBrokerServer * ptrServer = NULL;
bool serverInitialized = false;
PortableServer::POA_var server_poa;

void server_sigint_handler_callback(int sig_type) {
	userlog(
			log4cxx::Level::getInfo(),
			loggerAtmiBrokerServer,
			(char*) "server_sigint_handler_callback Received shutdown signal: %d",
			sig_type);
	serverdone();
	abort();
}

int serverrun() {
	tperrno = 0;
	return ptrServer->block();
}

int serverinit() {
	tperrno = 0;
	int toReturn = 0;

	initializeLogger();

	if (ptrServer == NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverinit called");
		signal(SIGINT, server_sigint_handler_callback);
		ptrServer = new AtmiBrokerServer();
		if (!serverInitialized) {
			::serverdone();
			toReturn = -1;
			tperrno = TPESYSTEM;
		} else {
			userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer,
					(char*) "Server Running");
		}
	}
	return toReturn;
}

int serverdone() {
	tperrno = 0;
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "serverdone called");
	if (ptrServer) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "serverdone deleting Corba server");
		delete ptrServer;
		ptrServer = NULL;
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "serverdone deleted Corba server");
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
		serverConnection = NULL;
		realConnection = NULL;
		char* transportLibrary = AtmiBrokerEnv::get_instance()->getenv(
				(char*) "TransportLibrary");
		LOG4CXX_INFO(loggerAtmiBrokerServer,
				(char*) "Loading server transport: " << transportLibrary);
		connection_factory_t* connectionFactory =
				(connection_factory_t*) ::lookup_symbol(transportLibrary,
						"connectionFactory");
		if (connectionFactory != NULL) {
			serverConnection = connectionFactory->create_connection(
					(char*) "server");
			realConnection = ::initOrb(
					(char*) "serverAdministration");
			//realConnection = AtmiBrokerOTS::init_orb(
			//		(char*) "serverAdministration");
			
			AtmiBrokerServerXml aAtmiBrokerServerXml;
			aAtmiBrokerServerXml.parseXmlDescriptor(&serverInfo,
					(char*) "SERVER.xml");
			serverName = server;

			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "creating POAs for %s" << server);
			AtmiBrokerPoaFac* serverPoaFactory = realConnection->poaFactory;
			this->poa = serverPoaFactory->createServerPoa(
					realConnection->orbRef, server, realConnection->root_poa,
					realConnection->root_poa_manager);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "created POAs for %s" << server);
			
			PortableServer::ObjectId_var oid =
					PortableServer::string_to_ObjectId(server);
			poa->activate_object_with_id(oid, this);
			CORBA::Object_var tmp_ref = poa->create_reference_with_id(oid,
					"IDL:AtmiBroker/Server:1.0");

			CosNaming::Name * name = realConnection->default_ctx->to_name(
					serverName);
			realConnection->name_ctx->bind(*name, tmp_ref);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "server_init(): finished.");

			serverInitialized = true;
		} else {
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not load the transport: "
							<< transportLibrary);
			tperrno = TPESYSTEM;
		}
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer,
				(char*) "serverinit - Unexpected CORBA exception: %s",
				e._name());
		tperrno = TPESYSTEM;
	}
}

// ~AtmiBrokerServer destructor.
//
AtmiBrokerServer::~AtmiBrokerServer() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "destructor");
	if (serverConnection) {
		server_done();
		delete serverConnection;
		serverConnection = NULL;
		shutdownBindings(realConnection);
		delete realConnection;
		realConnection = NULL;
	}

	serviceData.clear();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleted service array");

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleting services");
	AtmiBrokerMem::discard_instance();
	//TODO READD AtmiBrokerNotify::discard_instance();
	AtmiBrokerOTS::discard_instance();
	AtmiBrokerEnv::discard_instance();
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "deleted services");

	serverInitialized = false;
}

int AtmiBrokerServer::block() {

	int toReturn = 0;
	LOG4CXX_INFO(loggerAtmiBrokerServer, "Server waiting for requests...");
	try {
		this->realConnection->orbRef->run();
	} catch (CORBA::Exception& e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, "Unexpected CORBA exception: %s"
				<< e._name());
		toReturn = -1;
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer, "Unexpected exception");
		toReturn = -1;
	}
	return toReturn;
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

	if (realConnection->name_ctx) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "unadvertise "
				<< serverName);
		CosNaming::Name* name =
				realConnection->default_ctx->to_name(serverName);
		realConnection->name_ctx->unbind(*name);
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "unadvertised " << serverName);

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		char* svcname = (char*) serverInfo.serviceNames[i].c_str();
		if (isAdvertised(svcname)) {
			unadvertiseService(svcname);
		}
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "server_done(): returning.");
}

char *
AtmiBrokerServer::getServerName() {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServerName");
	return serverName;
}

bool AtmiBrokerServer::advertiseService(char * serviceName, void(*func)(
		TPSVCINFO *)) {
	if (!serviceName || strlen(serviceName) == 0) {
		tperrno = TPEINVAL;
		return false;
	}

	bool found = false;
	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		if (strncmp(serverInfo.serviceNames[i].c_str(), serviceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
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

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "advertiseService(): "
			<< serviceName);

	// create reference for Service Queue and cache
	try {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "create_service_queue: "
				<< serviceName);
		Destination* destination = serverConnection->createDestination(
				serviceName);
		addDestination(destination, func);
		LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "created destination: "
				<< serviceName);
	} catch (...) {
		LOG4CXX_ERROR(
				loggerAtmiBrokerServer,
				(char*) "service has already been advertised, however it appears to be by a different server (possibly with the same name), which is strange... "
						<< serviceName);
		tperrno = TPEMATCH;
		return false;
	}

	advertisedServices.push_back(serviceName);
	LOG4CXX_INFO(loggerAtmiBrokerServer, (char*) "advertised service "
			<< serviceName);
	toReturn = true;
	return toReturn;
}

void AtmiBrokerServer::unadvertiseService(char * serviceName) {
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i
			!= advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "remove_service_queue: " << serviceName);
			Destination * destination = removeDestination(serviceName);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "preparing to destroy" << serviceName);
			serverConnection->destroyDestination(destination);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "destroyed" << serviceName);
			advertisedServices.erase(i);
			LOG4CXX_INFO(loggerAtmiBrokerServer,
					(char*) "unadvertised service " << serviceName);
			break;
		}
	}
}

bool AtmiBrokerServer::isAdvertised(char * serviceName) {
	bool toReturn = false;
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i
			!= advertisedServices.end(); i++) {
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
	aServerInfo->securityType = CORBA::string_dup(
			serverInfo.securityType.c_str());
	aServerInfo->orbType = CORBA::string_dup(serverInfo.orbType.c_str());
	aServerInfo->queueSpaceName = CORBA::string_dup(queue_name);

	//aServerInfo->serviceNames.length(serverInfo.serviceNames.size());
	std::queue<AtmiBroker::octetSeq *> returnData;
	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		std::string serviceName = serverInfo.serviceNames[i];
		//AtmiBroker::octetSeq * aOctetSeq =
		returnData.push(new AtmiBroker::octetSeq(serviceName.length(),
				serviceName.length(), (unsigned char *) serviceName.c_str(),
				true));//*aOctetSeq));
	}
	AtmiBroker::ServiceNameSeq_var aOctetSeq = new AtmiBroker::ServiceNameSeq(
			serverInfo.serviceNames.size(), serverInfo.serviceNames.size(),
			returnData.front(), true);
	aServerInfo->serviceNames = aOctetSeq;

	return aServerInfo._retn();
}

// get_all_service_info() -- Implements IDL operation "AtmiBroker::Server::get_all_service_info".
//
AtmiBroker::ServiceInfoSeq*
AtmiBrokerServer::get_all_service_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "get_all_service_info()");

	AtmiBroker::ServiceInfoSeq_var aServiceInfoSeq =
			new AtmiBroker::ServiceInfoSeq();
	aServiceInfoSeq->length(serverInfo.serviceNames.size());

	int j = 0;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		AtmiBroker::ServiceInfo_var aServiceInfo =
				new AtmiBroker::ServiceInfo();
		aServiceInfo->serviceName = strdup((*i).serviceInfo.serviceName);
		aServiceInfo->poolSize = (*i).serviceInfo.poolSize;
		aServiceInfo->securityType = strdup((*i).serviceInfo.securityType);
		// ambiguous assignment on 32 bit machines - commenting out since this is the admin interface which is going to change
		//		aServiceInfoSeq[j] = aServiceInfo;
		j++;
	}
	return aServiceInfoSeq._retn();
}

// get_environment_variable_info() -- Implements IDL operation "AtmiBroker::Server::get_environment_variable_info".
//
AtmiBroker::EnvVariableInfoSeq*
AtmiBrokerServer::get_environment_variable_info()
		throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "get_environment_variable_info()");

	std::vector<envVar_t> & aEnvVarInfoSeq =
			AtmiBrokerEnv::get_instance()->getEnvVariableInfoSeq();

	AtmiBroker::EnvVariableInfoSeq* aEnvVariableInfoSeqPtr =
			new AtmiBroker::EnvVariableInfoSeq();

	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "get_environment_variable_info() setting length to "
					<< aEnvVarInfoSeq.size());
	aEnvVariableInfoSeqPtr->length(aEnvVarInfoSeq.size());
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "get_environment_variable_info() set length to "
					<< aEnvVariableInfoSeqPtr->length());
	int j = 0;
	for (std::vector<envVar_t>::iterator i = aEnvVarInfoSeq.begin(); i
			!= aEnvVarInfoSeq.end(); i++) {
		LOG4CXX_DEBUG(loggerAtmiBrokerServer,
				(char*) "get_environment_variable_info() env name "
						<< (*i).name);
		(*aEnvVariableInfoSeqPtr)[j].name = strdup((*i).name);
		(*aEnvVariableInfoSeqPtr)[j].value = strdup((*i).value);
		j++;
	}
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "get_environment_variable_info() returning");
	return aEnvVariableInfoSeqPtr;
}

// set_server_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_server_descriptor".
//
void AtmiBrokerServer::set_server_descriptor(const char* xml_descriptor)
		throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "set_server_descriptor() "
			<< xml_descriptor);

	std::string serverFileName = "server.xml";

	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_server_descriptor() file name "
					<< serverFileName.c_str());
	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_server_descriptor() file opened " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_server_descriptor() file written and closed");
}

// set_service_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_service_descriptor".
//
void AtmiBrokerServer::set_service_descriptor(const char* service_name,
		const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_service_descriptor() %s " << service_name << " "
					<< xml_descriptor);

	std::string serverFileName = "server.xml";

	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_service_descriptor()  file opened " << " "
					<< serverFileName.c_str() << " " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_service_descriptor()  file written and closed");
}

// set_environment_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_environment_descriptor".
//
void AtmiBrokerServer::set_environment_descriptor(const char* xml_descriptor)
		throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_environment_descriptor() " << xml_descriptor);

	FILE* aTempFile = fopen("Environment.xml", "w");
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_environment_descriptor() file Environment.xml opened "
					<< aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "set_environment_descriptor() file written and closed");
}

// stop_service() -- Implements IDL operation "AtmiBroker::Server::stop_service".
//
void AtmiBrokerServer::stop_service(const char* service_name)
		throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "stop_service() "
			<< service_name);

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "stop_service() next service "
							<< (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name,
					(const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(
						loggerAtmiBrokerServer,
						(char*) "stop_service() found matching service calling tpunadvertise"
								<< service_name);
#ifndef VBC_COMP
				tpunadvertise((char*) serverInfo.serviceNames[i].c_str());
#else
				tpunadvertise((char*)serverInfo.serviceNames[i]);
#endif
				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "stop_service() called tpunadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "stop_service() DID NOT find matching service "
						<< service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "stop_service() Could not stop service - exception: "
						<< e._name());
	}
}

// start_service() -- Implements IDL operation "AtmiBroker::Server::start_service".
//
void AtmiBrokerServer::start_service(const char* service_name)
		throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "start_service() "
			<< service_name);

	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "start_service()  stopping service first");
	stop_service(service_name);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer,
			(char*) "start_service()  stopped service");

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "start_service()  next service "
							<< (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name,
					(const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(
						loggerAtmiBrokerServer,
						(char*) "start_service()  found matching service calling tpadvertise"
								<< service_name);
#ifndef VBC_COMP
				tpadvertise((char*) serverInfo.serviceNames[i].c_str(), NULL);
#else
				tpadvertise((char*)serverInfo.serviceNames[i], NULL);
#endif
				LOG4CXX_DEBUG(loggerAtmiBrokerServer,
						(char*) "start_service()  called tpadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBrokerServer,
				(char*) "start_service()  DID NOT find matching service "
						<< service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(
				loggerAtmiBrokerServer,
				(char*) "start_service()  Could not start service - exception: "
						<< e._name());
	}
}

void AtmiBrokerServer::addDestination(Destination* destination, void(*func)(
		TPSVCINFO *)) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "addDestination: "
			<< destination->getName());

	ServiceData entry;
	entry.destination = destination;
	entry.func = func;

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "constructor: "
			<< destination->getName());
	entry.serviceInfo.poolSize = 1; // TODO MAKE A CONSTANT

	AtmiBrokerServiceXml aAtmiBrokerServiceXml;
	aAtmiBrokerServiceXml.parseXmlDescriptor(&entry.serviceInfo,
			destination->getName());

	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "createPool");
	for (int i = 0; i < entry.serviceInfo.poolSize; i++) {
		ServiceDispatcher* dispatcher = new ServiceDispatcher(destination,
				serverConnection, destination->getName(), func);
		if (dispatcher->activate(THR_NEW_LWP | THR_JOINABLE, 1, 0,
				ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
			delete dispatcher;
			LOG4CXX_ERROR(loggerAtmiBrokerServer,
					(char*) "Could not start thread pool");
		} else {
			entry.dispatchers.push_back(dispatcher);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) " destination "
					<< destination);
		}
	}

	serviceData.push_back(entry);
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "added: "
			<< destination->getName());
}

Destination* AtmiBrokerServer::removeDestination(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removeDestination: "
			<< aServiceName);
	Destination* toReturn = NULL;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
			!= serviceData.end(); i++) {
		if (strncmp((*i).destination->getName(), aServiceName,
				XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).destination;
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removing service "
					<< aServiceName);
			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				ServiceDispatcher* dispatcher = (*j);
				dispatcher->shutdown();
			}

			// TODO NOTIFY ALL REQUIRED HERE
			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end(); j++) {
				toReturn->disconnect();
			}

			for (std::vector<ServiceDispatcher*>::iterator j =
					(*i).dispatchers.begin(); j != (*i).dispatchers.end();) {
				ServiceDispatcher* dispatcher = (*j);
				j = (*i).dispatchers.erase(j);
				dispatcher->wait();
				delete dispatcher;
			}

			serviceData.erase(i);
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "removed: "
					<< aServiceName);
			break;
		}
	}
	return toReturn;
}

void (*AtmiBrokerServer::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "getServiceMethod: "
					<< aServiceName);

			for (std::vector<ServiceData>::iterator i = serviceData.begin(); i
					!= serviceData.end(); i++) {
				if (strncmp((*i).destination->getName(), aServiceName,
						XATMI_SERVICE_NAME_LENGTH) == 0) {
					LOG4CXX_DEBUG(loggerAtmiBrokerServer, (char*) "found: "
							<< aServiceName);
					return (*i).func;
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBrokerServer,
					(char*) "getServiceMethod out: " << aServiceName);
			return NULL;
		}
