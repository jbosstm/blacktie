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

#ifndef AtmiBroker_SERVICE_FAC_MGR_H_
#define AtmiBroker_SERVICE_FAC_MGR_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerS.h"
#elif ORBIX_COMP
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include "AtmiBroker_s.hh"
#endif

#include "xatmi.h"

struct _service_factory_data {
	CORBA::String_var serviceName;
	AtmiBroker::ServiceFactory_var factoryPtr;
	void (*func)(TPSVCINFO *);
};
typedef _service_factory_data ServiceFactoryData;

struct _service_manager_data {
	CORBA::String_var serviceName;
	AtmiBroker::ServiceManager_var managerPtr;

};
typedef _service_manager_data ServiceManagerData;

class ATMIBROKER_DLL AtmiBrokerServiceFacMgr {

public:

	AtmiBrokerServiceFacMgr(int numFactories);

	~AtmiBrokerServiceFacMgr();
	void (*getServiceMethod(const char * aServiceName))(TPSVCINFO *);
	AtmiBroker::ServiceFactory_ptr getServiceFactory(const char * aServiceName);
	AtmiBroker::ServiceManager_ptr getServiceManager(const char * aServiceName);

	void addServiceFactory(const char * aServiceName, const AtmiBroker::ServiceFactory_var& refPtr, void(*func)(TPSVCINFO *));
	void addServiceManager(const char * aServiceName, const AtmiBroker::ServiceManager_var& refPtr);

	void removeServiceFactory(const char * aServiceName);
	void removeServiceManager(const char * aServiceName);

	static AtmiBrokerServiceFacMgr* get_instance();
	static AtmiBrokerServiceFacMgr* get_instance(int numFactories);
	static void discard_instance();

private:

	ServiceFactoryData * serviceFactoryDataArray;
	int numServiceFactories;

	ServiceManagerData * serviceManagerDataArray;
	int numServiceManagers;

	static AtmiBrokerServiceFacMgr* ptrAtmiBrokerServiceFacMgr;
};

#endif
