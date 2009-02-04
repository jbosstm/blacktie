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
// Servant which implements the AtmiBrokerXA::ResourceManagerFactory interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include <stdlib.h>
#include <iostream>
#include "AtmiBrokerXA_ResourceManagerFactoryImpl.h"

#include "AtmiBrokerOTS.h"

#include "log4cxx/logger.h"


log4cxx::LoggerPtr loggerAtmiBrokerXA_ResourceManagerFactoryImpl(log4cxx::Logger::getLogger("AtmiBrokerXA_ResourceManagerFactoryImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBrokerXA::ResourceManagerFactory*
AtmiBrokerXA_ResourceManagerFactoryImpl::_create(PortableServer::POA_ptr the_poa) {
	return new AtmiBrokerXA_ResourceManagerFactoryImpl(the_poa);
}

// AtmiBrokerXA_ResourceManagerFactoryImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBrokerXA_ResourceManagerFactoryImpl::AtmiBrokerXA_ResourceManagerFactoryImpl(PortableServer::POA_ptr the_poa)
//:
// TODO READD  IT_ServantBaseOverrides(the_poa)
{
	// Initialise instance variables used for attributes
	//
}

// ~AtmiBrokerXA_ResourceManagerFactoryImpl destructor.
//
AtmiBrokerXA_ResourceManagerFactoryImpl::~AtmiBrokerXA_ResourceManagerFactoryImpl() {
	// Intentionally empty.
	//
}

// create_resource_manager() -- Implements IDL operation "AtmiBrokerXA::ResourceManagerFactory::create_resource_manager".
//
XA::ResourceManager_ptr AtmiBrokerXA_ResourceManagerFactoryImpl::create_resource_manager(const char * resource_manager_name, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) throw (CORBA::SystemException ) {
	LOG4CXX_LOGLS(loggerAtmiBrokerXA_ResourceManagerFactoryImpl, log4cxx::Level::getDebug(), (char*) "create_resource_manager()");

	XA::ResourceManager_ptr aXAResourceManager = AtmiBrokerOTS::get_instance()->getXaConnector()->create_resource_manager(resource_manager_name, AtmiBrokerOTS::get_instance()->getXaosw(),// xa switch
			open_string, // open string specified in config
			close_string, // close string specified in config,
			thread_model, // thread model is THREAD or PROCESShread model
			automatic_association, // no automatic association
			dynamic_registration_optimization, // do not use dynamic registration
			AtmiBrokerOTS::get_instance()->getXaCurrentConnection());

	LOG4CXX_LOGLS(loggerAtmiBrokerXA_ResourceManagerFactoryImpl, log4cxx::Level::getInfo(), (char*) "created XA Resource Manager: " << (void*) aXAResourceManager);
	LOG4CXX_LOGLS(loggerAtmiBrokerXA_ResourceManagerFactoryImpl, log4cxx::Level::getInfo(), (char*) "created XA Current Connection: " << (void*) AtmiBrokerOTS::get_instance()->getXaCurrentConnection());
	return aXAResourceManager;
}

// create_resource_manager_ior() -- Implements IDL operation "AtmiBrokerXA::ResourceManagerFactory::create_resource_manager".
//
char *
AtmiBrokerXA_ResourceManagerFactoryImpl::create_resource_manager_ior(const char * resource_manager_name, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization) throw (CORBA::SystemException ) {
	LOG4CXX_LOGLS(loggerAtmiBrokerXA_ResourceManagerFactoryImpl, log4cxx::Level::getError(), (char*) "create_resource_manager_ior() - NOT IMPLEMENTED");
	return NULL; // TEMP  create_resource_manager(resource_manager_name, open_string, close_string, thread_model, automatic_association, dynamic_registration_optimization);
}

