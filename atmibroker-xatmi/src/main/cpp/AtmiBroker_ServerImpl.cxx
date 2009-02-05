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
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include "AtmiBroker_ServerImpl.h"

#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerServerXml.h"
#include "AtmiBrokerEnv.h"
#include "userlog.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerAtmiBroker_ServerImpl(log4cxx::Logger::getLogger("AtmiBroker_ServerImpl"));

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
AtmiBroker_ServerImpl::AtmiBroker_ServerImpl(PortableServer::POA_ptr the_poa) {
	// Intentionally empty.
	std::string serverFileName = "SERVER.xml";
	AtmiBrokerServerXml aAtmiBrokerServerXml;
	aAtmiBrokerServerXml.parseXmlDescriptor(&serverInfo, serverFileName.c_str());
	serverName = server;
}

// ~AtmiBroker_ServerImpl destructor.
//
AtmiBroker_ServerImpl::~AtmiBroker_ServerImpl() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "destructor ");
	serviceData.clear();
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "deleted service array ");
}

// server_init() -- Implements IDL operation "AtmiBroker::Server::server_init".
//
CORBA::Short AtmiBroker_ServerImpl::server_init() throw (CORBA::SystemException ) {
	int toReturn = 0;
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_init(): called.");

	PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(server);
	server_poa->activate_object_with_id(oid, this);
	CORBA::Object_var tmp_ref = server_poa->create_reference_with_id(oid, "IDL:AtmiBroker/Server:1.0");

	CosNaming::Name * name = server_default_context->to_name(serverName);
	try {
		server_name_context->bind(*name, tmp_ref);
		AtmiBroker::Server_var pServer = AtmiBroker::Server::_narrow(tmp_ref);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_init(): finished.");
	} catch (CosNaming::NamingContext::AlreadyBound& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "server_init - Unexpected Already Bound exception: %s", server);
		toReturn = -1;
	}
	return toReturn;
}

// server_done() -- Implements IDL operation "AtmiBroker::Server::server_done".
//
void AtmiBroker_ServerImpl::server_done() throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_done()");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "unadvertise %s", serverName);

	CosNaming::Name * name;

	name = server_default_context->to_name(serverName);
	server_name_context->unbind(*name);

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "unadvertised %s", serverName);

	for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
		char* svcname = (char*) serverInfo.serviceNames[i].c_str();
		if (isAdvertised(svcname)) {
			unadvertiseService(svcname);
		}
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "server_done(): returning.");
}

char *
AtmiBroker_ServerImpl::getServerName() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServerName ");
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

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "advertiseService(): '%s'", serviceName);

	// create reference for Service Queue and cache
	try {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "create_service_queue: %s", serviceName);

		// create Poa for Service Queue
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "create_service_queue_poa: %s", serviceName);
		PortableServer::POA_ptr aFactoryPoaPtr = serverPoaFactory->createServiceFactoryPoa(server_orb, serviceName, server_poa, server_root_poa_manager);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "created create_service_factory_poa: %s", serviceName);

		ServiceQueue *tmp_factory_servant = new ServiceQueue(aFactoryPoaPtr, serviceName, func);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) " tmp_factory_servant %p", (void*) tmp_factory_servant);

		aFactoryPoaPtr->activate_object(tmp_factory_servant);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "activated tmp_servant %p", (void*) tmp_factory_servant);
		CORBA::Object_var tmp_ref = aFactoryPoaPtr->servant_to_reference(tmp_factory_servant);

		CosNaming::Name * name = server_default_context->to_name(serviceName);
		server_name_context->bind(*name, tmp_ref);

		addServiceQueue(serviceName, tmp_factory_servant, func);
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "created ServiceQueue %s", serviceName);

	} catch (...) {
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "service has already been advertised, however it appears to be by a different server (possibly with the same name), which is strange... %s", serviceName);
		tperrno = TPEMATCH;
		return false;
	}

	advertisedServices.push_back(serviceName);
	userlog(log4cxx::Level::getInfo(), loggerAtmiBroker_ServerImpl, (char*) "advertised service %s", serviceName);
	toReturn = true;
	return toReturn;
}

// TODO CLEANUP SERVICE POA AND SERVANT CACHE
void AtmiBroker_ServerImpl::unadvertiseService(char * serviceName) {
	for (std::vector<char*>::iterator i = advertisedServices.begin(); i != advertisedServices.end(); i++) {
		if (strcmp(serviceName, (*i)) == 0) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "remove_service_queue: %s", serviceName);
			CosNaming::Name * name = server_default_context->to_name(serviceName);
			server_name_context->unbind(*name);

			ServiceQueue* toDelete = removeServiceQueue(serviceName);
			PortableServer::POA_ptr poa = (PortableServer::POA_ptr) toDelete->getPoa();
			delete toDelete;
			poa->destroy(true, true);
			poa = NULL;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) " service Queue %s removed", serviceName);
			advertisedServices.erase(i);
			userlog(log4cxx::Level::getInfo(), loggerAtmiBroker_ServerImpl, (char*) "unadvertised service %s", serviceName);
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
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_server_info()");

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
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_all_service_info()");

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
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info()");

	std::vector<envVar_t> & aEnvVarInfoSeq = AtmiBrokerEnv::get_instance()->getEnvVariableInfoSeq();

	AtmiBroker::EnvVariableInfoSeq* aEnvVariableInfoSeqPtr = new AtmiBroker::EnvVariableInfoSeq();

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() setting length to %d", aEnvVarInfoSeq.size());
	aEnvVariableInfoSeqPtr->length(aEnvVarInfoSeq.size());
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() set length to %d", aEnvVariableInfoSeqPtr->length());
	int j = 0;
	for (std::vector<envVar_t>::iterator i = aEnvVarInfoSeq.begin(); i != aEnvVarInfoSeq.end(); i++) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() env name %s ", (*i).name);
		(*aEnvVariableInfoSeqPtr)[j].name = strdup((*i).name);
		(*aEnvVariableInfoSeqPtr)[j].value = strdup((*i).value);
		j++;
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "get_environment_variable_info() returning ");
	return aEnvVariableInfoSeqPtr;
}

// set_server_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_server_descriptor".
//
void AtmiBroker_ServerImpl::set_server_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() %s", xml_descriptor);

	std::string serverFileName = "server.xml";

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file name %s", serverFileName.c_str());
	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file opened %p", aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_server_descriptor() file written and closed ");
}

// set_service_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_service_descriptor".
//
void AtmiBroker_ServerImpl::set_service_descriptor(const char* service_name, const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor() %s %s", service_name, xml_descriptor);

	std::string serverFileName = "server.xml";

	FILE* aTempFile = fopen(serverFileName.c_str(), "w");
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file %s opened %p", serverFileName.c_str(), aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_service_descriptor()  file written and closed ");
}

// set_environment_descriptor() -- Implements IDL operation "AtmiBroker::Server::set_environment_descriptor".
//
void AtmiBroker_ServerImpl::set_environment_descriptor(const char* xml_descriptor) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() %s", xml_descriptor);

	FILE* aTempFile = fopen("Environment.xml", "w");
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file Environment.xml opened %p", aTempFile);

	fputs(xml_descriptor, aTempFile);

	fflush(aTempFile);
	fclose(aTempFile);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "set_environment_descriptor() file written and closed ");
}

// stop_service() -- Implements IDL operation "AtmiBroker::Server::stop_service".
//
void AtmiBroker_ServerImpl::stop_service(const char* service_name) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() %s", service_name);

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() next service %s", (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() found matching service %s...calling tpunadvertise", service_name);
#ifndef VBC_COMP
				tpunadvertise((char*) serverInfo.serviceNames[i].c_str());
#else
				tpunadvertise((char*)serverInfo.serviceNames[i]);
#endif
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() called tpunadvertise");
				return;
			}
		}
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() DID NOT find matching service %s...", service_name);
	} catch (CORBA::Exception &e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "stop_service() Could not stop service - exception: %s...", e._name());
	}
}

// start_service() -- Implements IDL operation "AtmiBroker::Server::start_service".
//
void AtmiBroker_ServerImpl::start_service(const char* service_name) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service() %s", service_name);

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopping service first ");
	stop_service(service_name);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  stopped service");

	try {
		for (unsigned int i = 0; i < serverInfo.serviceNames.size(); i++) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  next service %s", (const char*) serverInfo.serviceNames[i].c_str());
			if (strcmp(service_name, (const char*) serverInfo.serviceNames[i].c_str()) == 0) {
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  found matching service %s...calling tpadvertise", service_name);
#ifndef VBC_COMP
				tpadvertise((char*) serverInfo.serviceNames[i].c_str(), NULL);
#else
				tpadvertise((char*)serverInfo.serviceNames[i], NULL);
#endif
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  called tpadvertise");
				return;
			}
		}
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  DID NOT find matching service %s...", service_name);
	} catch (CORBA::Exception &e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBroker_ServerImpl, (char*) "start_service()  Could not start service - exception: %s...", e._name());
	}
}

ServiceQueue* AtmiBroker_ServerImpl::getServiceQueue(const char * aServiceName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue: %s", aServiceName);

	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "found: %s", (char*) (*i).serviceName);
			return (*i).serviceQueue;
		}
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue out: %s", aServiceName);
	return NULL;
}

void AtmiBroker_ServerImpl::addServiceQueue(char*& aServiceName, ServiceQueue*& aFactoryPtr, void(*func)(TPSVCINFO *)) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "addServiceQueue: %s", aServiceName);

	ServiceData entry;
	entry.serviceName = aServiceName;
	entry.serviceQueue = aFactoryPtr;
	entry.func = func;
	serviceData.push_back(entry);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "added: %s", (char*) aServiceName);
}

ServiceQueue* AtmiBroker_ServerImpl::removeServiceQueue(const char * aServiceName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "removeServiceQueue: %s", aServiceName);
	ServiceQueue* toReturn = NULL;
	for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).serviceQueue;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "removing service %s", (char*) (*i).serviceName);
			serviceData.erase(i);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "removed: %s", aServiceName);
			break;
		}
	}
	return toReturn;
}

void (*AtmiBroker_ServerImpl::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServiceQueue: %s", aServiceName);

			for (std::vector<ServiceData>::iterator i = serviceData.begin(); i != serviceData.end(); i++) {
				if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "found: %s", (char*) (*i).serviceName);
					return (*i).func;
				}
			}
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker_ServerImpl, (char*) "getServiceMethod out: %s", aServiceName);
			return NULL;
		}
