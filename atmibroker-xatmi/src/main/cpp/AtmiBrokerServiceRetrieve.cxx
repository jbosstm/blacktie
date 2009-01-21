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
#include <orbsvcs/CosNamingS.h>
#elif ORBIX_COMP
#include <omg/orb.hh>
#include <omg/CosNaming.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#include <CosNaming_c.hh>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "AtmiBrokerClient.h"

#include "AtmiBrokerServiceRetrieve.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServiceRetrieve(Logger::getLogger("AtmiBrokerServiceRetrieve"));

void get_server(const char * serverName, AtmiBroker::Server_ptr * aServicePtrAddr) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "get_server: %s", serverName);

	CosNaming::Name * name = client_default_context->to_name(serverName);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Server name %p", (void*) name);
	CORBA::Object_var tmp_ref = client_name_context->resolve(*name);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Server ref %p", (void*) tmp_ref);

	CORBA::String_var ior = client_orb->object_to_string(tmp_ref);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Server ior %s", (char*) ior);

	//	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "validating connection ");
	//#ifdef TAO_COMP
	//	CORBA::Boolean aBoolean = tmp_ref->_validate_connection(policyList);
	//#else
	//#elif ORBIX_COMP
	//	CORBA::Boolean aBoolean = tmp_ref->_validate_connection(*policyList);
	//#endif
	//	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "validated connection %d", aBoolean);

	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "narrowing %p", (void*) tmp_ref);
#ifndef VBC_COMP
	*aServicePtrAddr = AtmiBroker::Server::_unchecked_narrow(tmp_ref);
#else
	*aServicePtrAddr = AtmiBroker::Server::_narrow(tmp_ref);
#endif

	if (CORBA::is_nil(*aServicePtrAddr)) {
		userlog(Level::getError(), loggerAtmiBrokerServiceRetrieve, (char*) "Could not retrieve Server %s", serverName);
	} else
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "retrieved  %s Server %p", serverName, (void*) *aServicePtrAddr);
}

AtmiBroker::ServiceFactory_ptr get_service_factory(const char * serviceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "get_service_factory: %s", serviceName);

	CosNaming::Name * name = client_default_context->to_name(serviceName);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Service name %p", (void*) name);

	AtmiBroker::ServiceFactory_ptr factoryPtr = NULL;

	try {
		CORBA::Object_var tmp_ref = client_name_context->resolve(*name);
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Service ref %p", (void*) tmp_ref);
		factoryPtr = AtmiBroker::ServiceFactory::_narrow(tmp_ref);
	} catch (...) {
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Could not access the service factory %p", (void*) name);
	}

	if (CORBA::is_nil(factoryPtr)) {
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "Could not retrieve Factory for %s", serviceName);
	} else
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "retrieved  %s Factory %p", serviceName, (void*) factoryPtr);

	return AtmiBroker::ServiceFactory::_duplicate(factoryPtr);
}

void get_service(long clientId, AtmiBroker::ServiceFactory_ptr aFactoryPtr, char **idPtr, AtmiBroker::Service_var* refPtr) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "get_service");

	CORBA::String_var aId = "";
	CORBA::String_var aIor = "";

	// get service reference from factory
	//*refPtr 	= aFactoryPtr->get_service(aConversation, aId);

	// get service ior from factory
	aIor = aFactoryPtr->get_service_id(clientId, aId);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "got back ior %s", (char*) aIor);

	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "string_to_object ");
	CORBA::Object_var tmp_ref = client_orb->string_to_object(aIor);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "tmp_ref %p", (void*) tmp_ref);

	*refPtr = AtmiBroker::Service::_narrow(tmp_ref);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "refPtr %p", (void*) *refPtr);

	if (CORBA::is_nil(*refPtr)) {
		userlog(Level::getError(), loggerAtmiBrokerServiceRetrieve, (char*) "Could not get service ");
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "got service %p with id %s", (void*) *refPtr, (char*) aId);
		strcpy(*idPtr, (char*) aId);
	}
}

void find_service(long clientId, AtmiBroker::ServiceFactory_ptr aFactoryPtr, char *id, AtmiBroker::Service_var* refPtr) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "find_service - id: %s", id);

	CORBA::String_var aIor = "";

	// find service reference in factory
	//*refPtr 	= aFactoryPtr->find_service(id);

	// find service ior in factory
	aIor = aFactoryPtr->find_service_id(clientId, id);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceRetrieve, (char*) "got back ior %s", (char*) aIor);

	CORBA::Object_var tmp_ref = client_orb->string_to_object(aIor);
	*refPtr = AtmiBroker::Service::_narrow(tmp_ref);

	if (CORBA::is_nil(*refPtr)) {
		userlog(Level::getError(), loggerAtmiBrokerServiceRetrieve, (char*) "Could not get service ");
	} else {
		userlog(Level::getInfo(), loggerAtmiBrokerServiceRetrieve, (char*) "got service %p with id %s", (void*) *refPtr, id);
	}
}
