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
#include "tao/ORB_Core.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "AtmiBrokerOTS.h"
#include "AtmiBrokerPoaFac.h"

#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerPoaFac(Logger::getLogger("AtmiBrokerPoaFac"));

AtmiBrokerPoaFac::AtmiBrokerPoaFac() {
	// Intentionally empty.
}

AtmiBrokerPoaFac::~AtmiBrokerPoaFac() {
	// Intentionally empty.
	//
}

// createCallbackPoa() -- Create a POA using a ServantActivator.
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createCallbackPoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(1);
	int i = 0;

	// Make the POA single threaded.
	//
	policies[i++] = parent_poa->create_thread_policy(PortableServer::SINGLE_THREAD_MODEL);

	//assert(i==1);

	//	try {
	return parent_poa->create_POA(poa_name, poa_manager, policies);
	/*	}
	 catch (PortableServer::POA::AdapterAlreadyExists)
	 {
	 return parent_poa->find_POA(poa_name, true);
	 }
	 */return PortableServer::POA::_nil();
}

// createServiceFactoryPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServiceFactoryPoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;

	// Create Persistant Lifespan
	//
	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);

	// Make the POA single threaded.
	//
	policies[i++] = parent_poa->create_thread_policy(PortableServer::SINGLE_THREAD_MODEL);

	//    assert(i==2);

	try {
		return parent_poa->create_POA(poa_name, poa_manager, policies);
	} catch (PortableServer::POA::AdapterAlreadyExists) {
		userlog(Level::getError(), loggerAtmiBrokerPoaFac, (char*) "createServiceFactoryPoa already existed: %s", poa_name);
		return parent_poa->find_POA(poa_name, true);
	}
	return PortableServer::POA::_nil();
}

// createServiceManagerPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServiceManagerPoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;

	// Create Persistant Lifespan
	//
	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);

	// Make the POA single threaded.
	//
	policies[i++] = parent_poa->create_thread_policy(PortableServer::SINGLE_THREAD_MODEL);

	//    assert(i==2);

	try {
		return parent_poa->create_POA(poa_name, poa_manager, policies);
	} catch (PortableServer::POA::AdapterAlreadyExists) {
		userlog(Level::getError(), loggerAtmiBrokerPoaFac, (char*) "createServiceManagerPoa already existed: %s", poa_name);
		return parent_poa->find_POA(poa_name, true);
	}
	return PortableServer::POA::_nil();
}

// createServicePoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServicePoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;

	// Make the POA single threaded.
	//
	policies[i++] = parent_poa->create_thread_policy(PortableServer::ORB_CTRL_MODEL);

	// Create Transactional ADAPTS policy
	//
	policies[i++] = AtmiBrokerOTS::get_instance()->getTransactionPolicy();

	//    assert(i==2);

	try {
		return parent_poa->create_POA(poa_name, poa_manager, policies);
	} catch (PortableServer::POA::AdapterAlreadyExists) {
		userlog(Level::getError(), loggerAtmiBrokerPoaFac, (char*) "createServicePoa already existed: %s", poa_name);
		return parent_poa->find_POA(poa_name, true);
	}
	return PortableServer::POA::_nil();
}

// createServerPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServerPoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;

	// Create Persistant Lifespan
	//
	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);

	// Create User Id
	//
	policies[i++] = parent_poa->create_id_assignment_policy(PortableServer::USER_ID);

	//try {
	return parent_poa->create_POA(poa_name, poa_manager, policies);
	/*	}
	 catch (PortableServer::POA::AdapterAlreadyExists)
	 {
	 return parent_poa->find_POA(poa_name, true);
	 }
	 */return PortableServer::POA::_nil();
}

