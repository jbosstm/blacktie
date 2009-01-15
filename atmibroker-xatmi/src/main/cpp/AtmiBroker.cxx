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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>
#include <sstream>

#include "OrbManagement.h"

//TODO READD #include "extensions.h"
extern "C" {
#include "AtmiBrokerClientControl.h"
}

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerServiceFacMgr.h"
#include "AtmiBrokerMem.h"
#include "AtmiBroker.h"
#include "AtmiBrokerConversation.h"
//TODO READD #include "AtmiBrokerNotify.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerOTS.h"

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker(Logger::getLogger("AtmiBroker"));

bool clientInitialized;
int _tperrno = 0;
long _tpurcode = -1;

// ORB and Naming Service References

CORBA::ORB_var server_orb;
PortableServer::POA_var server_root_poa;
PortableServer::POAManager_var server_root_poa_manager;
CosNaming::NamingContextExt_var server_default_context;
CosNaming::NamingContext_var server_name_context;
PortableServer::POA_var server_poa;
AtmiBrokerPoaFac * serverPoaFactory;
CORBA::ORB_var client_orb;
PortableServer::POA_var client_root_poa;
PortableServer::POAManager_var client_root_poa_manager;
CosNaming::NamingContextExt_var client_default_context;
CosNaming::NamingContext_var client_name_context;
PortableServer::POA_var client_poa;
AtmiBrokerPoaFac * clientPoaFactory;

// Memory management
AtmiBrokerMem * ptrAtmiBrokerMem;
void (*callbackFunctionPtr)(const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, const char * id);

void (*callbackTypedBufferFunctionPtr)(const AtmiBroker::TypedBuffer& idata, CORBA::Long ilen, CORBA::Long flags, const char * id);

// Corba Client
AtmiBrokerClient * ptrAtmiBrokerClient;

// Client Policies
CORBA::PolicyList clientPolicies;
//CORBA::PolicyManager_var policyManager;
CORBA::PolicyTypeSeq policyTypes;
CORBA::PolicyList *policyList;

// Constants
//int                                             SERVICE_ID_SIZE         	= 40;
const char* SERVER_LIT = "SERVER:";
const char* MAX_REPLICAS = "MAX_REPLICAS:";
int MAX_SERVERS = 10;
int MAX_SERVICES = 20;
int MAX_SERVICE_CACHE_SIZE = 10;
int MIN_SERVICE_CACHE_SIZE = 5;
int MIN_AVAILABLE_SERVICE_CACHE_SIZE = 2;
int MAX_COMPANY_SIZE = 20;

//TODO -NOT IMPLEMENTED int                                             maxReplicas			= 1;

// runtime arguments
void createClientCallbackPOA();
void createClientBindingPolicies();
void destroyAllLogs();

void shutdownBindings(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager, CosNaming::NamingContextExt_var& ctx, CosNaming::NamingContext_var& nameCtx, PortableServer::POA_var& innerPoa) {

	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleting Service Factory Manager ");
	AtmiBrokerServiceFacMgr::discard_instance();
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleted Service Factory Manager ");
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleting Service Factory Manager ");
	//TODO READD AtmiBrokerNotify::discard_instance();
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleted Service Factory Manager ");
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleting AtmiBrokerOTS");
	AtmiBrokerOTS::discard_instance();
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings deleted AtmiBrokerOTS");

	userlog(Level::getInfo(), loggerAtmiBroker, "Closing Bindings");

	if (!CORBA::is_nil(orbRef)) {
		userlog(Level::getDebug(), loggerAtmiBroker, "shutdownBindings shutting down ORB ");
		try {
			orbRef->shutdown(1);
			userlog(Level::getDebug(), loggerAtmiBroker, "shutdownBindings shut down ORB ");
		} catch (CORBA::Exception &ex) {
			userlog(Level::getError(), loggerAtmiBroker, (char*) "shutdownBindings Unexpected CORBA exception shutting down orb: %s", ex._name());
		} catch (...) {
			userlog(Level::getFatal(), loggerAtmiBroker, (char*) "shutdownBindings Unexpected fatal exception");
		}
		//}
		//if (!CORBA::is_nil(orbRef)) {
		try {
			// TODO DOES NOT WORK WHEN NO ORB WORK DONE
			userlog(Level::getDebug(), loggerAtmiBroker, (char*) "shutdownBindings destroying ORB ");
			orbRef->destroy();
		} catch (CORBA::Exception &ex) {
			userlog(Level::getError(), loggerAtmiBroker, (char*) "shutdownBindings Unexpected CORBA exception destroying orb: %s", ex._name());
		}
	}

	// TODO
	//	if (innerPoa) {
	//innerPoa->destroy(true, true);
	//CORBA::release(innerPoa);
	innerPoa = NULL;
	//	}
	//	if (ctx) {
	//CORBA::release(ctx);
	ctx = NULL;
	//	}
	//	if (nameCtx) {
	//CORBA::release(nameCtx);
	nameCtx = NULL;
	//	}
	//	if (poa_manager) {
	//poa_manager->deactivate();
	//CORBA::release(poa_manager);
	poa_manager = NULL;
	//	}
	//	if (poa) {
	//poa->destroy(true, true);
	//CORBA::release(poa);
	poa = NULL;
	//	}
	orbRef = NULL;

	AtmiBrokerEnv::discard_instance();

	userlog(Level::getInfo(), loggerAtmiBroker, (char*) "Closed Bindings");
}

int clientinit() {
	if (!clientInitialized) {
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientinit called");
		_tperrno = 0;

		signal(SIGINT, client_termination_handler_callback);

		if (AtmiBrokerEnv::get_instance()->getenv("LOG4CXXCONFIG") != NULL) {
			PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv("LOG4CXXCONFIG"));
		} else {
			BasicConfigurator::configure();
		}

		try {
			initOrb("client", client_orb);
			getNamingServiceAndContext(client_orb, client_default_context, client_name_context);

			getRootPOAAndManager(client_orb, client_root_poa, client_root_poa_manager);
			createClientCallbackPOA();
			createClientBindingPolicies();
			//AtmiBrokerOTS::get_instance()->setCorbaObjects(client_orb, client_default_context, client_name_context);
			//AtmiBrokerOTS::get_instance()->getTransactionCurrent();
			AtmiBrokerMem::get_instance();

			userlog(Level::getDebug(), loggerAtmiBroker, (char*) " about to create service factory manager ");
			AtmiBrokerServiceFacMgr::get_instance(MAX_SERVICES);
			userlog(Level::getDebug(), loggerAtmiBroker, (char*) " created service factory manager ");

			ptrAtmiBrokerClient = new AtmiBrokerClient(true, false, true, false);

			clientInitialized = true;
		} catch (CORBA::Exception &ex) {
			userlog(Level::getError(), loggerAtmiBroker, (char*) "clientinit Unexpected CORBA exception: %s", ex._name());
			tperrno = TPESYSTEM;

			// TODO CLEAN UP CALLBACKPOA, TRANSACTION CURRENT, LOG FACTORY

			// CLEAN UP INITIALISED ITEMS
			AtmiBrokerConversation::discard_instance();
			if (ptrAtmiBrokerClient) {
				userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleting Corba Client ");
				delete ptrAtmiBrokerClient;
				ptrAtmiBrokerClient = NULL;
				userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleted Corba Client ");
			}

			AtmiBrokerMem::get_instance()->freeAllMemory();

			shutdownBindings(client_orb, client_root_poa, client_root_poa_manager, client_default_context, client_name_context, client_poa);
			return -1;
		}
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientinit finished");
		return 1;
	}
	return 0;
}

int clientdone() {
	_tperrno = 0;
	if (clientInitialized) {
		userlog(Level::getInfo(), loggerAtmiBroker, (char*) "clientdone called");

		AtmiBrokerConversation::discard_instance();
		if (ptrAtmiBrokerClient) {
			userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleting Corba Client ");
			delete ptrAtmiBrokerClient;
			userlog(Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleted Corba Client ");
		}

		AtmiBrokerMem::get_instance()->freeAllMemory();

		shutdownBindings(client_orb, client_root_poa, client_root_poa_manager, client_default_context, client_name_context, client_poa);

		clientInitialized = false;
		userlog(Level::getInfo(), loggerAtmiBroker, (char*) "clientdone returning");
	}
	return 0;
}


int * _get_tperrno(void) {
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "_get_tperrno");
	return &_tperrno;
}

long * _get_tpurcode(void) {
	userlog(Level::getError(), loggerAtmiBroker, (char*) "_get_tpurcode - Not implemented");
	return &_tpurcode;
}

// client_termination_handler_callback() -- handle fatal signals/events gracefully.
//
void client_termination_handler_callback(int sig_type) {
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "client_termination_handler_callback Received shutdown signal: %d", sig_type);

	// TODO no abort
	if (!CORBA::is_nil(client_orb)) {
		clientdone();
	} else {
		userlog(Level::getWarn(), loggerAtmiBroker, (char*) "client_termination_handler_callback ORB not initialised, aborting.");
		abort();
	}
}

void getRootPOAAndManager(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager) {
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager");

	CORBA::Object_var tmp_ref;

	if (CORBA::is_nil(poa)) {
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager resolving the root POA ");
		tmp_ref = orbRef->resolve_initial_references("RootPOA");
		poa = PortableServer::POA::_narrow(tmp_ref);
		//assert(!CORBA::is_nil(poa));
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager resolved the root POA: %p", (void*) poa);
	} else {
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager already resolved the root POA: %p", (void*) poa);
	}

	if (CORBA::is_nil(poa_manager)) {
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager getting the root POA manager");
		poa_manager = poa->the_POAManager();
		//assert(!CORBA::is_nil(poa_manager));
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getRootPOAAndManager got the root POA manager: %p", (void*) poa_manager);
	} else {
		userlog(Level::getError(), loggerAtmiBroker, (char*) "getRootPOAAndManager already resolved the root POA manager: %p", (void*) poa_manager);
	}
}

void createClientCallbackPOA() {
	userlog(Level::getDebug(), loggerAtmiBroker, (char*) "createClientCallbackPOA createClientCallbackPOA");

	if (CORBA::is_nil(client_poa)) {
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "createClientCallbackPOA creating POA with name %s", server);
		clientPoaFactory = new AtmiBrokerPoaFac();
		std::string name = ".client";
		name.insert(0, server);
		client_poa = clientPoaFactory->createCallbackPoa(name.c_str(), client_root_poa, client_root_poa_manager);
		userlog(Level::getDebug(), loggerAtmiBroker, (char*) "createClientCallbackPOA created POA: %p", (void*) client_poa);
	} else
		userlog(Level::getError(), loggerAtmiBroker, (char*) "createClientCallbackPOA already created POA: %p", (void*) client_poa);
}

void createClientBindingPolicies() {
	userlog(Level::getWarn(), loggerAtmiBroker, (char*) "createClientBindingPolicies - Not implemented");

	/*********
	 CORBA::Object_var 	tmp_ref;  // For temporary object references.

	 if(CORBA::is_nil(policyManager))
	 {
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "getting policy manager ");
	 tmp_ref 		= client_orb->resolve_initial_references("ORBPolicyManager");
	 policyManager 		= CORBA::PolicyManager::_narrow(tmp_ref);
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "got policy manager %p", (void*)policyManager);

	 policyList = new CORBA::PolicyList();
	 policyList->length(2);
	 CORBA::Any policyValueB;
	 CORBA::Any policyValueI;

	 IT_CORBA::BindingEstablishmentPolicyValue valb;
	 valb.relative_expiry		= (TimeBase::TimeT)200000000;
	 valb.max_binding_iterations	= (CORBA::UShort)50;
	 valb.max_forwards		= (CORBA::UShort)50;
	 valb.initial_iteration_delay	= (TimeBase::TimeT)2000000;
	 valb.backoff_ratio		= (CORBA::Float)2;
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "val is %p", (void*)&valb);

	 policyValueB <<= valb;
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "policyValue is %p", (void*)&policyValueB);

	 *policyList[0] =   client_orb->create_policy(IT_CORBA::BINDING_ESTABLISHMENT_POLICY_ID, policyValueB);

	 IT_CORBA::InvocationRetryPolicyValue vali;
	 vali.max_retries		= (CORBA::UShort)50;
	 vali.max_rebinds		= (CORBA::UShort)50;
	 vali.max_forwards		= (CORBA::UShort)50;
	 vali.initial_retry_delay	= (TimeBase::TimeT)2000000;
	 vali.backoff_ratio		= (CORBA::Float)2;
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "val is %p", (void*)&vali);

	 policyValueI <<= vali;
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "policyValue is %p", (void*)&policyValueI);

	 *policyList[1] =   client_orb->create_policy(IT_CORBA::INVOCATION_RETRY_POLICY_ID, policyValueI);

	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "setting policy list for binding  establishment %p", (void*)*policyList[0]);
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "setting policy list for invocation retries %p", (void*)*policyList[1]);
	 policyManager->set_policy_overrides(*policyList, CORBA::ADD_OVERRIDE);
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "set policy list for binding  establishment %p", (void*)*policyList[0]);
	 userlog(Level::getDebug(), loggerAtmiBroker, (char*) "set policy list for invocation retries %p", (void*)*policyList[1]);
	 }
	 ********/
}

