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
