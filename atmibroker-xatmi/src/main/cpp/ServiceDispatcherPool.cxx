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
// Servant which implements the AtmiBroker::ServiceFactory interface.
//
#include "ServiceDispatcherPool.h"
#include "xatmi.h"

log4cxx::LoggerPtr ServiceDispatcherPool::logger(log4cxx::Logger::getLogger("ServiceDispatcherPool"));

// Constants
int MAX_SERVICE_CACHE_SIZE = 1;

// ServiceDispatcherPool constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
ServiceDispatcherPool::ServiceDispatcherPool(CORBA_CONNECTION* connection, Destination* destination, char *serviceName, void(*func)(TPSVCINFO *)) {
	this->serviceName = serviceName;
	LOG4CXX_DEBUG(logger, (char*) "constructor: " << serviceName);
	serviceInfo.poolSize = MAX_SERVICE_CACHE_SIZE;

	char* serviceConfigFilename = (char*) malloc(XATMI_SERVICE_NAME_LENGTH + 10);
	memset(serviceConfigFilename, '\0', XATMI_SERVICE_NAME_LENGTH + 10);
	strncpy(serviceConfigFilename, serviceName, XATMI_SERVICE_NAME_LENGTH);
	strcat(serviceConfigFilename, ".xml");
	LOG4CXX_DEBUG(logger, (char*) "loading: " << serviceConfigFilename);
	
	AtmiBrokerServiceXml aAtmiBrokerServiceXml;
	aAtmiBrokerServiceXml.parseXmlDescriptor(&serviceInfo, serviceConfigFilename);
	free(serviceConfigFilename);

	this->destination = destination;

	LOG4CXX_DEBUG(logger, (char*) "createPool");
	for (int i = 0; i < MAX_SERVICE_CACHE_SIZE; i++) {
		ServiceDispatcher* dispatcher = new ServiceDispatcher(connection, this->destination, serviceName, func);
		if (dispatcher->activate(THR_NEW_LWP| THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
			delete dispatcher;
			LOG4CXX_ERROR(logger, (char*) "Could not start thread pool");
		} else {
			dispatchers.push_back(dispatcher);
		}
	}
	LOG4CXX_DEBUG(logger, (char*) "createPool done ");
}

// ~ServiceDispatcherPool destructor.
//
ServiceDispatcherPool::~ServiceDispatcherPool() {

	for (std::vector<ServiceDispatcher*>::iterator i = dispatchers.begin(); i != dispatchers.end(); i++) {
		ServiceDispatcher* dispatcher = (*i);
		dispatcher->shutdown();
	}

	// TODO NOTIFY ALL REQUIRED HERE
	for (std::vector<ServiceDispatcher*>::iterator i = dispatchers.begin(); i != dispatchers.end(); i++) {
		destination->disconnect();
	}

	std::vector<ServiceDispatcher*>::iterator i = dispatchers.begin();
	while (i != dispatchers.end()) {
		ServiceDispatcher* dispatcher = (*i);
		i = dispatchers.erase(i);
		dispatcher->wait();
		delete dispatcher;
	}
}

SVCINFO ServiceDispatcherPool::get_service_info() {
	LOG4CXX_DEBUG(logger, (char*) "get_service_info()");
	SVCINFO svcinfo;
	svcinfo.serviceName = strdup(serviceName);
	svcinfo.poolSize = serviceInfo.poolSize;
	svcinfo.securityType = strdup("");
	return svcinfo;
}

Destination* ServiceDispatcherPool::getDestination() {
	return destination;
}
