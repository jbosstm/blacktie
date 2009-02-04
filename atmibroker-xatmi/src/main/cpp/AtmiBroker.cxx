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

#include "xatmi.h"

#include "OrbManagement.h"

#include "AtmiBrokerClientControl.h"

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerMem.h"
#include "AtmiBroker.h"
#include "AtmiBrokerConversation.h"
//TODO READD #include "AtmiBrokerNotify.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerOTS.h"

#include "Sender.h"
#include "SenderImpl.h"

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"


log4cxx::LoggerPtr loggerAtmiBroker(log4cxx::Logger::getLogger("AtmiBroker"));

bool loggerInitialized;
bool clientInitialized;

// ORB and Naming Service References
Worker* client_worker;
CORBA::ORB_var client_orb;
PortableServer::POA_var client_root_poa;
PortableServer::POAManager_var client_root_poa_manager;
CosNaming::NamingContextExt_var client_default_context;
CosNaming::NamingContext_var client_name_context;
PortableServer::POA_var client_poa;
AtmiBrokerPoaFac * clientPoaFactory;

// Memory management
AtmiBrokerMem * ptrAtmiBrokerMem;

// Corba Client
AtmiBrokerClient * ptrAtmiBrokerClient;

// Client Policies
CORBA::PolicyList clientPolicies;
//CORBA::PolicyManager_var policyManager;
CORBA::PolicyTypeSeq policyTypes;
CORBA::PolicyList *policyList;

void client_sigint_handler_callback(int sig_type) {
	userlog(log4cxx::Level::getWarn(), loggerAtmiBroker, (char*) "client_sigint_handler_callback Received shutdown signal: %d", sig_type);
	clientdone();
	abort();
}

int clientinit() {
	if (!clientInitialized) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit called");
		_tperrno = 0;

		signal(SIGINT, client_sigint_handler_callback);

		if (!loggerInitialized) {
			if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
				log4cxx::PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
			} else {
				log4cxx::BasicConfigurator::configure();
			}
			loggerInitialized = true;
		}

		try {
			AtmiBrokerOTS::init_orb((char*) "client", client_worker, client_orb, client_default_context, client_name_context);

			getRootPOAAndManager(client_orb, client_root_poa, client_root_poa_manager);

			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "createClientCallbackPOA creating POA with name client");
			clientPoaFactory = new AtmiBrokerPoaFac();
			std::string name = ".client";
			//			name.insert(0, server);
			client_poa = clientPoaFactory->createCallbackPoa(client_orb, name.c_str(), client_root_poa, client_root_poa_manager);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "createClientCallbackPOA created POA: %p", (void*) client_poa);

			client_root_poa_manager->activate();
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "activated poa - started processing requests ");

			ptrAtmiBrokerClient = new AtmiBrokerClient();

			clientInitialized = true;

			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "Client Initialized");
			return 1;
		} catch (CORBA::Exception &ex) {
			userlog(log4cxx::Level::getError(), loggerAtmiBroker, (char*) "clientinit Unexpected CORBA exception: %s", ex._name());
			tperrno = TPESYSTEM;

			// TODO CLEAN UP CALLBACKPOA, TRANSACTION CURRENT, LOG FACTORY

			// CLEAN UP INITIALISED ITEMS
			AtmiBrokerConversation::discard_instance();
			if (ptrAtmiBrokerClient) {
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleting Corba Client ");
				delete ptrAtmiBrokerClient;
				ptrAtmiBrokerClient = NULL;
				userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleted Corba Client ");
			}

			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleting services");
			AtmiBrokerMem::discard_instance();
			//TODO READD AtmiBrokerNotify::discard_instance();
			AtmiBrokerOTS::discard_instance();
			AtmiBrokerEnv::discard_instance();
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleted services");

			shutdownBindings(client_orb, client_root_poa, client_root_poa_manager, client_default_context, client_name_context, client_poa, client_worker);
			return -1;
		}
	}
	return 0;
}

int clientdone() {
	_tperrno = 0;
	if (clientInitialized) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone called");

		AtmiBrokerConversation::discard_instance();
		if (ptrAtmiBrokerClient) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleting Corba Client ");
			delete ptrAtmiBrokerClient;
			ptrAtmiBrokerClient = NULL;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleted Corba Client ");
		}

		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleting services");
		AtmiBrokerMem::discard_instance();
		//TODO READD AtmiBrokerNotify::discard_instance();
		AtmiBrokerOTS::discard_instance();
		AtmiBrokerEnv::discard_instance();
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone deleted services");

		shutdownBindings(client_orb, client_root_poa, client_root_poa_manager, client_default_context, client_name_context, client_poa, client_worker);

		clientInitialized = false;
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "Client Shutdown");
	}
	return 0;
}

Sender* get_service_queue(const char * serviceName) {
	return new SenderImpl(client_default_context, client_name_context, serviceName);
}
