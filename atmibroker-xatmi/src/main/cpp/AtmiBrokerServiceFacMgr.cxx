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
	return ptrAtmiBrokerServiceFacMgr;
}

void AtmiBrokerServiceFacMgr::discard_instance() {
	if (ptrAtmiBrokerServiceFacMgr != NULL) {
		delete ptrAtmiBrokerServiceFacMgr;
		ptrAtmiBrokerServiceFacMgr = NULL;
	}
}

AtmiBrokerServiceFacMgr *
AtmiBrokerServiceFacMgr::get_instance(int aServiceFactoryNumber) {
	if (ptrAtmiBrokerServiceFacMgr == NULL)
		ptrAtmiBrokerServiceFacMgr = new AtmiBrokerServiceFacMgr(aServiceFactoryNumber);
	return ptrAtmiBrokerServiceFacMgr;
}

AtmiBrokerServiceFacMgr::AtmiBrokerServiceFacMgr(int aServiceFactoryNumber) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "constructor %d", aServiceFactoryNumber);

	numServiceFactories = aServiceFactoryNumber;
	numServiceManagers = aServiceFactoryNumber;

	serviceFactoryDataArray = new ServiceFactoryData[numServiceFactories];

	for (int i = 0; i < numServiceFactories; i++) {
		serviceFactoryDataArray[i].serviceName = (char*) NULL;
		serviceFactoryDataArray[i].factoryPtr = NULL;
	}

	serviceManagerDataArray = new ServiceManagerData[numServiceManagers];

	for (int j = 0; j < numServiceFactories; j++) {
		serviceManagerDataArray[j].serviceName = (char*) NULL;
		serviceManagerDataArray[j].managerPtr = NULL;
	}
}

AtmiBrokerServiceFacMgr::~AtmiBrokerServiceFacMgr() {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "destructor ");

	if (serviceFactoryDataArray) {
		userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleting service factory array %p", (void*) serviceFactoryDataArray);
		delete[] serviceFactoryDataArray;
		serviceFactoryDataArray = NULL;
		userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleted service factory array ");
	}

	if (serviceManagerDataArray) {
		userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleting service manager array %p", (void*) serviceManagerDataArray);
		delete[] serviceManagerDataArray;
		serviceManagerDataArray = NULL;
		userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "deleted service manager array ");
	}
}

AtmiBroker::ServiceFactory_ptr AtmiBrokerServiceFacMgr::getServiceFactory(const char * aServiceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %d", numServiceFactories);

	for (int i = 0; i < numServiceFactories; i++) {
		if (serviceFactoryDataArray[i].serviceName != NULL && strcmp(serviceFactoryDataArray[i].serviceName, aServiceName) == 0) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) serviceFactoryDataArray[i].serviceName);
			return serviceFactoryDataArray[i].factoryPtr;
		}
	}
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager out: %s", aServiceName);
	return (AtmiBroker::ServiceFactory*) NULL;
}

void
(*AtmiBrokerServiceFacMgr::getServiceMethod(const char * aServiceName))(TPSVCINFO *)
		{
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceFactory: %d", numServiceFactories);

			for (int i = 0; i < numServiceFactories; i++) {
				if (serviceFactoryDataArray[i].serviceName != NULL && strcmp(serviceFactoryDataArray[i].serviceName, aServiceName) == 0) {
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) serviceFactoryDataArray[i].serviceName);
					return serviceFactoryDataArray[i].func;
				}
			}
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager out: %s", aServiceName);
			return NULL;
		}

		AtmiBroker::ServiceManager_ptr AtmiBrokerServiceFacMgr::getServiceManager(const char * aServiceName) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "getServiceManager: %s", aServiceName);

			for (int i = 0; i < numServiceManagers; i++) {
				if (serviceManagerDataArray[i].serviceName != NULL && strcmp(serviceManagerDataArray[i].serviceName, aServiceName) == 0) {
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "found: %s", (char*) serviceManagerDataArray[i].serviceName);
					return serviceManagerDataArray[i].managerPtr;
				}
			}
			return (AtmiBroker::ServiceManager*) NULL;
		}

		void AtmiBrokerServiceFacMgr::addServiceFactory(const char * aServiceName, const AtmiBroker::ServiceFactory_var& aFactoryPtr, void(*func)(TPSVCINFO *)) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "addServiceFactory: %s", aServiceName);

			for (int i = 0; i < numServiceFactories; i++) {
				if (serviceFactoryDataArray[i].serviceName == NULL) {
					serviceFactoryDataArray[i].serviceName = aServiceName;
					serviceFactoryDataArray[i].factoryPtr = aFactoryPtr;
					serviceFactoryDataArray[i].func = func;
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "added: %s at %d", (char*) serviceFactoryDataArray[i].serviceName, i);
					return;
				}
			}
		}

		void AtmiBrokerServiceFacMgr::addServiceManager(const char * aServiceName, const AtmiBroker::ServiceManager_var& aManagerPtr) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "addServiceManager: %s", aServiceName);

			for (int i = 0; i < numServiceManagers; i++) {
				if (serviceManagerDataArray[i].serviceName == NULL) {
					serviceManagerDataArray[i].serviceName = aServiceName;
					serviceManagerDataArray[i].managerPtr = aManagerPtr;
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "added: %s at %d", (char*) serviceManagerDataArray[i].serviceName, i);
					return;
				}
			}
		}

		void AtmiBrokerServiceFacMgr::removeServiceFactory(const char * aServiceName) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removeServiceFactory: %s", aServiceName);

			for (int i = 0; i < numServiceFactories; i++) {
				if (serviceFactoryDataArray[i].serviceName != NULL && strcmp(serviceFactoryDataArray[i].serviceName, aServiceName) == 0) {
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removing service %s", (const char*) serviceFactoryDataArray[i].serviceName);
					serviceFactoryDataArray[i].serviceName = (char *) NULL;
					serviceFactoryDataArray[i].factoryPtr = NULL;
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removed: %s at %d", aServiceName, i);
					return;
				}
			}
		}

		void AtmiBrokerServiceFacMgr::removeServiceManager(const char * aServiceName) {
			userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removeServiceManager: %s", aServiceName);

			for (int i = 0; i < numServiceManagers; i++) {
				if (serviceManagerDataArray[i].serviceName != NULL && strcmp(serviceManagerDataArray[i].serviceName, aServiceName) == 0) {
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removing service %s", (const char*) serviceManagerDataArray[i].serviceName);
					serviceManagerDataArray[i].serviceName = (char*) NULL;
					serviceManagerDataArray[i].managerPtr = NULL;
					userlog(Level::getDebug(), loggerAtmiBrokerServiceFacMgr, (char*) "removed: %s at %d", aServiceName, i);
					return;
				}
			}
		}

