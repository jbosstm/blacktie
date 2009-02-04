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


log4cxx::LoggerPtr loggerAtmiBrokerServiceFacMgr(log4cxx::Logger::getLogger("AtmiBrokerServiceFacMgr"));

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
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "constructor");
}

AtmiBrokerServiceFacMgr::~AtmiBrokerServiceFacMgr() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "destructor ");

	serviceFactoryData.clear();
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleted service factory array ");
}

ServiceQueue* AtmiBrokerServiceFacMgr::getServiceFactory(const char * aServiceName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %s", aServiceName);

	for (std::vector<ServiceFactoryData>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) (*i).serviceName);
			return (*i).factoryPtr;
		}
	}
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory out: %s", aServiceName);
	return NULL;
}

void AtmiBrokerServiceFacMgr::addServiceFactory(char*& aServiceName, ServiceQueue*& aFactoryPtr, void(*func)(TPSVCINFO *)) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "addServiceFactory: %s", aServiceName);

	ServiceFactoryData entry;
	entry.serviceName = aServiceName;
	entry.factoryPtr = aFactoryPtr;
	entry.func = func;
	serviceFactoryData.push_back(entry);
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "added: %s", (char*) aServiceName);
}

ServiceQueue* AtmiBrokerServiceFacMgr::removeServiceFactory(const char * aServiceName) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removeServiceFactory: %s", aServiceName);
	ServiceQueue* toReturn = NULL;
	for (std::vector<ServiceFactoryData>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
		if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
			toReturn = (*i).factoryPtr;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removing service %s", (char*) (*i).serviceName);
			serviceFactoryData.erase(i);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removed: %s", aServiceName);
			break;
		}
	}
	return toReturn;
}

void (*AtmiBrokerServiceFacMgr::getServiceMethod(const char * aServiceName))(TPSVCINFO *) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %s", aServiceName);

			for (std::vector<ServiceFactoryData>::iterator i = serviceFactoryData.begin(); i != serviceFactoryData.end(); i++) {
				if (strncmp((*i).serviceName, aServiceName, XATMI_SERVICE_NAME_LENGTH) == 0) {
					userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) (*i).serviceName);
					return (*i).func;
				}
			}
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceMethod out: %s", aServiceName);
			return NULL;
		}
