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

#include "ServiceQueue.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerServiceFacMgr.h"
#include "AtmiBrokerServiceFac.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServiceFac(Logger::getLogger("AtmiBrokerServiceFac"));

void remove_service_factory(char* serviceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "remove_service_factory: %s", serviceName);
	CosNaming::Name * name = server_default_context->to_name(serviceName);
	server_name_context->unbind(*name);

	ServiceQueue* toDelete = AtmiBrokerServiceFacMgr::get_instance()->removeServiceFactory(serviceName);
	PortableServer::POA_ptr poa = toDelete->getPoa();
	delete toDelete;
	poa->destroy(true, true);
	poa = NULL;
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) " service factory %s removed", serviceName);
}

void create_service_factory(char *serviceName, void(*func)(TPSVCINFO *)) {
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "create_service_factory: %s", serviceName);

	// create Poa for Service Factory
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "create_service_factory_poa: %s", serviceName);
	PortableServer::POA_ptr aFactoryPoaPtr = serverPoaFactory->createServiceFactoryPoa(server_orb, serviceName, server_poa, server_root_poa_manager);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "created create_service_factory_poa: %s", serviceName);

	ServiceQueue *tmp_factory_servant = new ServiceQueue(aFactoryPoaPtr, serviceName, func);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) " tmp_factory_servant %p", (void*) tmp_factory_servant);

	aFactoryPoaPtr->activate_object(tmp_factory_servant);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "activated tmp_servant %p", (void*) tmp_factory_servant);
	CORBA::Object_var tmp_ref = aFactoryPoaPtr->servant_to_reference(tmp_factory_servant);

	CosNaming::Name * name = server_default_context->to_name(serviceName);
	server_name_context->bind(*name, tmp_ref);

	AtmiBrokerServiceFacMgr::get_instance()->addServiceFactory(serviceName, tmp_factory_servant, func);
	userlog(Level::getDebug(), loggerAtmiBrokerServiceFac, (char*) "created ServiceFactory %s", serviceName);
}
