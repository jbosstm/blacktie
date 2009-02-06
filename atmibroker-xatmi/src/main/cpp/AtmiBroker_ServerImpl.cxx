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
#include "AtmiBroker_ServerImpl.h"
#include "AtmiBroker.h"
#include "AtmiBrokerPoaFac.h"
#include "AtmiBrokerEnv.h"
#include "log4cxx/logger.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr loggerAtmiBroker_ServerImpl(log4cxx::Logger::getLogger("AtmiBroker_ServerImpl"));

// AtmiBroker_ServerImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ServerImpl::AtmiBroker_ServerImpl(CONNECTION* connection, PortableServer::POA_ptr poa) {
	AtmiBrokerServerXml aAtmiBrokerServerXml;
	aAtmiBrokerServerXml.parseXmlDescriptor(&serverInfo, (char*) "SERVER.xml");
	serverName = server;
	this->connection = connection;
	this->poa = poa;
}

// ~AtmiBroker_ServerImpl destructor.
//
AtmiBroker_ServerImpl::~AtmiBroker_ServerImpl() {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "destructor ");
	serviceData.clear();
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "deleted service array ");
}

// server_init() -- Implements IDL operation "AtmiBroker::Server::server_init".
//
CORBA::Short AtmiBroker_ServerImpl::server_init() throw (CORBA::SystemException ) {
	int toReturn = 0;
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "server_init(): called.");

	PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(server);
	poa->activate_object_with_id(oid, this);
	CORBA::Object_var tmp_ref = poa->create_reference_with_id(oid, "IDL:AtmiBroker/Server:1.0");

	CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serverName);
	((CosNaming::NamingContext_ptr) connection->name_ctx)->bind(*name, tmp_ref);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "server_init(): finished.");
	return toReturn;
}

// server_done() -- Implements IDL operation "AtmiBroker::Server::server_done".
//
void AtmiBroker_ServerImpl::server_done() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "server_done()");

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "unadvertise " << serverName);

	CosNaming::Name* name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serverName);
	((CosNaming::NamingContext_ptr) connection->name_ctx)->unbind(*name);

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "unadvertised " << serverName);

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		char* svcname = (char*) serverInfo.serviceNames[i].c_str();
		if (isAdvertised(svcname)) {
			unadvertiseService(svcname);
		}
	}
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "server_done(): returning.");
}

char *
AtmiBroker_ServerImpl::getServerName() {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "getServerName ");
	return serverName;
}

bool AtmiBroker_ServerImpl::advertiseService(char * serviceName, void(*func)(TPSVCINFO *)) {
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

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "advertiseService(): " << serviceName);

	// create reference for Service Queue and cache
	try {
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "create_service_queue: " << serviceName);

		// create Poa for Service Queue
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "create_service_queue_poa: " << serviceName);
		AtmiBrokerPoaFac* poaFactory = ((AtmiBrokerPoaFac*) connection->poaFactory);
		PortableServer::POA_ptr aFactoryPoaPtr = poaFactory->createServicePoa((CORBA::ORB_ptr) connection->orbRef, serviceName, poa, (PortableServer::POAManager_ptr) connection->root_poa_manager);
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "created create_service_factory_poa: " << serviceName);

		Destination* destination = ::create_service_queue(serverConnection, aFactoryPoaPtr, serviceName);
		ServiceQueue *tmp_factory_servant = new ServiceQueue(serverConnection, destination, serviceName, func);
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) " tmp_factory_servant " << tmp_factory_servant);

		addServiceQueue(serviceName, tmp_factory_servant, func);
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "created ServiceQueue " << serviceName);

	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBroker_ServerImpl, (char*) "service has already been advertised, however it appears to be by a different server (possibly with the same name), which is strange... " << serviceName);
		tperrno = TPEMATCH;
		return false;
	}

	advertisedServices.push_back(serviceName);
	LOG4CXX_INFO(loggerAtmiBroker_ServerImpl, (char*) "advertised service " << serviceName);
	toReturn = true;
	return toReturn;
}

// TODO CLEANUP SERVICE POA AND SERVANT CACHE
void AtmiBroker_ServerImpl::unadvertiseService(char * serviceName) {
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "remove_service_queue: " << serviceName);
			CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serviceName);
			((CosNaming::NamingContext_ptr) connection->name_ctx)->unbind(*name);

			ServiceQueue* toDelete = removeServiceQueue(serviceName);
			EndpointQueue* queue = dynamic_cast<EndpointQueue*> (toDelete->getDestination());
			PortableServer::POA_ptr poa = (PortableServer::POA_ptr) queue->getPoa();
			delete toDelete;
			poa->destroy(true, true);
			poa = NULL;
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "service Queue removed" << serviceName);
			advertisedServices.erase(i);
			LOG4CXX_INFO(loggerAtmiBroker_ServerImpl, (char*) "unadvertised service " << serviceName);
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

// get_server_info() -- Implements IDL operation "AtmiBroker::Server::get_server_info".
//
AtmiBroker::ServerInfo*
AtmiBroker_ServerImpl::get_server_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_server_info()");

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
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_all_service_info()");

	AtmiBroker::ServiceInfoSeq_var aServiceInfoSeq = new AtmiBroker::ServiceInfoSeq();
	aServiceInfoSeq->length(serverInfo.serviceNames.size());

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		SVCINFO svcInfo = getServiceQueue((char*) serverInfo.serviceNames[i].c_str())->get_service_info();
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
AtmiBroker_ServerImpl::get_environment_variable_info() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info()");

	std::vector<envVar_t> & aEnvVarInfoSeq = AtmiBrokerEnv::get_instance()->getEnvVariableInfoSeq();

	AtmiBroker::EnvVariableInfoSeq* aEnvVariableInfoSeqPtr = new AtmiBroker::EnvVariableInfoSeq();

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() setting length to " << aEnvVarInfoSeq.size());
	aEnvVariableInfoSeqPtr->length(aEnvVarInfoSeq.size());
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() set length to " << aEnvVariableInfoSeqPtr->length());
	int j = 0;
	for (std::vector<envVar_t>::iterator i = aEnvVarInfoSeq.begin(); i != aEnvVarInfoSeq.end(); i++) {
		LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() env name " << (*i).name);
		(*aEnvVariableInfoSeqPtr)[j].name = strdup((*i).name);
		(*aEnvVariableInfoSeqPtr)[j].value = strdup((*i).value);
		j++;
	}
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() returning ");
	return aEnvVariableInfoSeqPtr;
}

// set_server_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_server_descriptor".
//
void AtmiBroker_ServerImpl::set_server_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() " << xml_descriptor);

	std::string serverFileName = "server.xml";

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file name " << serverFileName.c_str());
	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file opened " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file written and closed ");
}

// set_service_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_service_descriptor".
//
void AtmiBroker_ServerImpl::set_service_descriptor(const char* service_name, const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor() %s " << service_name << " " << xml_descriptor);

	std::string serverFileName = "server.xml";

	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file opened " << " " << serverFileName.c_str() << " " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file written and closed ");
}

// set_environment_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_environment_descriptor".
//
void AtmiBroker_ServerImpl::set_environment_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() " << xml_descriptor);

	FILE* aTempFile = fopen("Environment.xml", "w");
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file Environment.xml opened " << aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file written and closed ");
}

// stop_service() -- Implements IDL operation "AtmiBroker::Server::stop_service".
//
void AtmiBroker_ServerImpl::stop_service(const char* service_name) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "stop_service() " << service_name);

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "stop_service() next service " << (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "stop_service() found matching service calling tpunadvertise" << service_name);
#ifndef VBC_COMP
				tpunadvertise((char*) serverInfo.serviceNames[i].c_str());
#else
				tpunadvertise((char*)serverInfo.serviceNames[i]);
#endif
				LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "stop_service() called tpunadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBroker_ServerImpl, (char*) "stop_service() DID NOT find matching service " << service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(loggerAtmiBroker_ServerImpl, (char*) "stop_service() Could not stop service - exception: " << e._name());
	}
}

// start_service() -- Implements IDL operation "AtmiBroker::Server::start_service".
//
void AtmiBroker_ServerImpl::start_service(const char* service_name) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service() " << service_name);

	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopping service first ");
	stop_service(service_name);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopped service");

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service()  next service " << (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service()  found matching service calling tpadvertise" << service_name);
#ifndef VBC_COMP
				tpadvertise((char*) serverInfo.serviceNames[i].c_str(), NULL);
#else
				tpadvertise((char*)serverInfo.serviceNames[i], NULL);
#endif
				LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "start_service()  called tpadvertise");
				return;
			}
		}
		LOG4CXX_ERROR(loggerAtmiBroker_ServerImpl, (char*) "start_service()  DID NOT find matching service " << service_name);
	} catch (CORBA::Exception &e) {
		LOG4CXX_ERROR(loggerAtmiBroker_ServerImpl, (char*) "start_service()  Could not start service - exception: " << e._name());
	}
}

ServiceQueue* AtmiBroker_ServerImpl::getServiceQueue(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue: " << aServiceName);

	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "found: " << (char*) (*i).serviceName);
			return (*i).serviceQueue;
		}
	}
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue out: " << aServiceName);
	return NULL;
}

void AtmiBroker_ServerImpl::addServiceQueue(char*& aServiceName, ServiceQueue*& aFactoryPtr, void(*func)(TPSVCINFO *)) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "addServiceQueue: " << aServiceName);

	ServiceData entry;
	entry.serviceName = aServiceName;
	entry.serviceQueue = aFactoryPtr;
	entry.func = func;
	serviceData.push_back(entry);
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "added: " << (char*) aServiceName);
}

ServiceQueue* AtmiBroker_ServerImpl::removeServiceQueue(const char * aServiceName) {
	LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "removeServiceQueue: " << aServiceName);
	ServiceQueue* toReturn = NULL;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).serviceQueue;
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "removing service " << (char*) (*i).serviceName);
			serviceData.erase(i);
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "removed: " << aServiceName);
			break;
		}
	}
	return toReturn;
}

void (*AtmiBroker_ServerImpl::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue: " << aServiceName);

			for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
				if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "found: " << (char*) (*i).serviceName);
					return (*i).func;
				}
			}
			LOG4CXX_DEBUG(loggerAtmiBroker_ServerImpl, (char*) "getServiceMethod out: " << aServiceName);
			return NULL;
		}
