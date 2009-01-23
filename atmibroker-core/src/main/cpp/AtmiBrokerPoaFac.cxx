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
#include "tao/AnyTypeCode/Any.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "TxPolicyS.h"
#include "AtmiBrokerPoaFac.h"

#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerPoaFac(Logger::getLogger("AtmiBrokerPoaFac"));

// install transaction policy on this POA
int add_transaction_policy(CORBA::ORB_var& orb, CORBA::PolicyList& policies, PortableServer::POA_ptr poa, int index, int maxindex) {
	if (maxindex - index < 1)
		return 0;

	//policies[index++] = poa->create_request_processing_policy(PortableServer::USE_SERVANT_MANAGER);
	CORBA::Any any;
	any <<= AtmiTx::ADAPTS;

	policies[index++] = orb->create_policy(AtmiTx::OTS_POLICY_TYPE, any);

	return 2;
}

AtmiBrokerPoaFac::AtmiBrokerPoaFac() {
	// Intentionally empty.
}

AtmiBrokerPoaFac::~AtmiBrokerPoaFac() {
	// Intentionally empty.
	//
}

// createCallbackPoa() -- Create a POA using a ServantActivator.
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createCallbackPoa(CORBA::ORB_var orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(1);
	//	int i = 0;

	// Make the POA single threaded.
	//
	//	policies[i++] = parent_poa->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
	//	policies[i++] = parent_poa->create_thread_policy(PortableServer::SINGLE_THREAD_MODEL);
	//
	// install transaction policy
	add_transaction_policy(orb, policies, parent_poa, 0, 1);

	//assert(i==1);

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

// createServiceFactoryPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServiceFactoryPoa(const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(1);
	int i = 0;

	// Create Persistant Lifespan
	//
	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);

	// Make the POA single threaded.
	//
	//	policies[i++] = parent_poa->create_thread_policy(PortableServer::SINGLE_THREAD_MODEL);

	//    assert(i==2);

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

// createServicePoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServicePoa(CORBA::ORB_var orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(3);
	int i = 0;

	// Make the POA single threaded.
	//
	policies[i++] = parent_poa->create_thread_policy(PortableServer::ORB_CTRL_MODEL);

	// install transaction policy
	add_transaction_policy(orb, policies, parent_poa, i, 3);
	//    assert(i==2);

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

// createServerPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServerPoa(CORBA::ORB_var orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
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

	// install transaction policy
	//add_transaction_policy(orb, policies, parent_poa, i, 3);

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

