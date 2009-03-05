/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

log4cxx::LoggerPtr loggerAtmiBrokerPoaFac(log4cxx::Logger::getLogger("AtmiBrokerPoaFac"));

// install transaction policy on this POA
void add_transaction_policy(CORBA::ORB_ptr& orb, CORBA::PolicyList& policies, PortableServer::POA_ptr poa, int& index, int maxindex) {
	if (maxindex - index < 1)
		return;

	//policies[index++] = poa->create_request_processing_policy(PortableServer::USE_SERVANT_MANAGER);
	CORBA::Any any;
	any <<= AtmiTx::ADAPTS;

	try {
		policies[index++] = orb->create_policy(AtmiTx::OTS_POLICY_TYPE, any);
	} catch (const ::CORBA::PolicyError& ex) {
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerPoaFac, (char*) "no policy factory for AtmiTx::OTS_POLICY_TYPE has been registered");
	} catch (...) {
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerPoaFac, (char*) "unexpected error whilst createing policy: AtmiTx::OTS_POLICY_TYPE");
		throw ; // don't know what to do about that
	}
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
PortableServer::POA_ptr AtmiBrokerPoaFac::createCallbackPoa(CORBA::ORB_ptr orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	CORBA::PolicyList policies;
	policies.length(0);
	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

// createServicePoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServicePoa(CORBA::ORB_ptr orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;
	policies[i++] = parent_poa->create_thread_policy(PortableServer::ORB_CTRL_MODEL);
	//	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);
	add_transaction_policy(orb, policies, parent_poa, i, policies.length());

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

// createServerPoa()
//
PortableServer::POA_ptr AtmiBrokerPoaFac::createServerPoa(CORBA::ORB_ptr orb, const char* poa_name, PortableServer::POA_ptr parent_poa, PortableServer::POAManager_ptr poa_manager) {
	// Create a policy list. Policies not set in the list get default values.
	//
	CORBA::PolicyList policies;
	policies.length(2);
	int i = 0;

	//	policies[i++] = parent_poa->create_lifespan_policy(PortableServer::PERSISTENT);
	policies[i++] = parent_poa->create_id_assignment_policy(PortableServer::USER_ID);

	return parent_poa->create_POA(poa_name, poa_manager, policies);
}

