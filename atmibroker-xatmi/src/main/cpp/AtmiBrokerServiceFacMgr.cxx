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

#include <iostream>

#include "AtmiBrokerServiceFacMgr.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServiceFacMgr(Logger::getLogger("AtmiBrokerServiceFacMgr"));

AtmiBrokerServiceFacMgr *AtmiBrokerServiceFacMgr::ptrAtmiBrokerServiceFacMgr = NULL;

AtmiBrokerServiceFacMgr *
AtmiBrokerServiceFacMgr::get_instance() {
	if (ptrAtmiBrokerServiceFacMgr == NULL) {
		ptrAtmiBrokerServiceFacMgr = new AtmiBrokerServiceFacMgr();
	}
	return ptrAtmiBrokerServiceFacMgr;
}

void AtmiBrokerServiceFacMgr::discard_instance() {
	if (ptrAtmiBrokerServiceFacMgr != NULL) {
		delete ptrAtmiBrokerServiceFacMgr;
		ptrAtmiBrokerServiceFacMgr = NULL;
	}
}

AtmiBrokerServiceFacMgr::AtmiBrokerServiceFacMgr() {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "constructor");
}

AtmiBrokerServiceFacMgr::~AtmiBrokerServiceFacMgr() {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "destructor ");

	serviceFactoryData.clear();
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleted service factory array ");

	serviceManagerData.clear();
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleted service manager array ");
}

AtmiBroker::ServiceFactory_ptr AtmiBrokerServiceFacMgr::getServiceFactory(const char * aServiceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %s", aServiceName);

	for (std::vector<ServiceFactoryData*>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
		if (strncmp((*i)->serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) (*i)->serviceName);
			return (*i)->factoryPtr;
		}
	}
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager out: %s", aServiceName);
	return NULL;
}

AtmiBroker::ServiceManager_ptr AtmiBrokerServiceFacMgr::getServiceManager(const char * aServiceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager: %s", aServiceName);

	for (std::vector<ServiceManagerData*>::iterator i = serviceManagerData.begin(); i != serviceManagerData.end(); i++) {
		if (strncmp((*i)->serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) (*i)->serviceName);
			return (*i)->managerPtr;
		}
	}
	return NULL;
}

void AtmiBrokerServiceFacMgr::addServiceFactory(const char * aServiceName, const AtmiBroker::ServiceFactory_var& aFactoryPtr, void(*func)(TPSVCINFO *)) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "addServiceFactory: %s", aServiceName);

	ServiceFactoryData entry;
	entry.serviceName = aServiceName;
	entry.factoryPtr = aFactoryPtr;
	entry.func = func;
	serviceFactoryData.push_back(&entry);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "added: %s", (char*) aServiceName);
}

void AtmiBrokerServiceFacMgr::addServiceManager(const char * aServiceName, const AtmiBroker::ServiceManager_var& aManagerPtr) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "addServiceManager: %s", aServiceName);

	ServiceManagerData entry;
	entry.serviceName = aServiceName;
	entry.managerPtr = aManagerPtr;
	serviceManagerData.push_back(&entry);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "added: %s", (char*) aServiceName);
}

void AtmiBrokerServiceFacMgr::removeServiceFactory(const char * aServiceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removeServiceFactory: %s", aServiceName);

	for (std::vector<ServiceFactoryData*>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
		if (strncmp((*i)->serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removing service %s", (char*) (*i)->serviceName);
			serviceFactoryData.erase(i);
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removed: %s", aServiceName);
			break;
		}
	}
}

void AtmiBrokerServiceFacMgr::removeServiceManager(const char * aServiceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removeServiceManager: %s", aServiceName);

	for (std::vector<ServiceManagerData*>::iterator i = serviceManagerData.begin(); i != serviceManagerData.end(); i++) {
		if (strncmp((*i)->serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removing service %s", (const char*) (*i)->serviceName);
			serviceManagerData.erase(i);
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removed: %s", aServiceName);
			break;
		}
	}
}

void (*AtmiBrokerServiceFacMgr::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %s", aServiceName);

			for (std::vector<ServiceFactoryData*>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
				if (strncmp((*i)->serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) (*i)->serviceName);
					return (*i)->func;
				}
			}
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager out: %s", aServiceName);
			return NULL;
		}
