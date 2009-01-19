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
#include <iostream>

#include "OrbManagement.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"
#include "tx.h"

#include "AtmiBroker.h"
#include "AtmiBrokerServiceFacMgr.h"
#include "AtmiBrokerServerFac.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerServer.h"
extern "C" {
#include "AtmiBrokerServerControl.h"
}
#include "AtmiBroker_ServerImpl.h"
//TODO READD #include "AtmiBrokerNotify.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServer(Logger::getLogger("AtmiBrokerServer"));

bool serverInitialized;

AtmiBroker_ServerImpl * ptrServer;
AtmiBrokerServerFac * ptrServerFactory;

Worker* server_worker;
CORBA::ORB_var server_orb;
PortableServer::POA_var server_root_poa;
PortableServer::POAManager_var server_root_poa_manager;
CosNaming::NamingContextExt_var server_default_context;
CosNaming::NamingContext_var server_name_context;
PortableServer::POA_var server_poa;
AtmiBrokerPoaFac * serverPoaFactory;

void createServerPOA();

void server_sigint_handler_callback(int sig_type) {
	userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "server_termination_handler_callback Received shutdown signal: %d", sig_type);

	if (!CORBA::is_nil(server_orb)) {
		serverdone();
	} else {
		userlog(Level::getWarn(), loggerAtmiBrokerServer, (char*) "server_termination_handler_callback ORB not initialised, aborting.");
		abort(); // TODO REMOVE ABORT
	}
}

void server_sigsegv_handler_callback(int sig_type) {
	LOG4CXX_ERROR(loggerAtmiBrokerServer, (char*) "TODO I AM A HACK");
}

int serverrun() {
	int toReturn = 0;
	try {
		userlog(Level::getInfo(), loggerAtmiBrokerServer, "serverrun Waiting for requests...");
		server_orb->run();
	} catch (CORBA::Exception& e) {
		userlog(Level::getInfo(), loggerAtmiBrokerServer, "serverrun Unexpected CORBA exception: %s", e._name());
		toReturn = -1;
	}
	return toReturn;
}

int serverinit(int argc, char ** argv) {
	if (!serverInitialized) {
		userlog(Level::getInfo(), loggerAtmiBrokerServer, (char*) "serverinit called");
		_tperrno = 0;
		signal(SIGINT, server_sigint_handler_callback);
		//		signal(SIGSEGV, server_sigsegv_handler_callback);

		if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
			PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
		} else {
			BasicConfigurator::configure();
		}

		try {
			initOrb((char*) "server", server_worker, server_orb);
			AtmiBrokerMem::get_instance();
			getRootPOAAndManager(server_orb, server_root_poa, server_root_poa_manager);
			getNamingServiceAndContext(server_orb, server_default_context, server_name_context);
			createServerPOA();

			AtmiBrokerOTS::get_instance()->getTransactionCurrent();
			AtmiBrokerOTS::get_instance()->createTransactionPolicy();

			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit about to create service factory manager ");
			AtmiBrokerServiceFacMgr::get_instance(MAX_SERVICES);
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit created service factory manager ");

			//TODO READD AtmiBrokerNotify::get_instance()->setPOAAndContext(server_root_poa, server_default_context);

			server_root_poa_manager->activate();
			userlog(Level::getInfo(), loggerAtmiBrokerServer, (char*) "activated poa - started processing requests ");

			ptrServerFactory = new AtmiBrokerServerFac();
			ptrServerFactory->createServer(argc, argv);

			serverInitialized = true;
		} catch (CORBA::Exception& e) {
			userlog(Level::getError(), loggerAtmiBrokerServer, (char*) "serverinit - Unexpected CORBA exception: %s", e._name());
			tperrno = TPESYSTEM;

			// TODO CLEAN UP TRANSACTION CURRENT
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleting services");
			AtmiBrokerMem::get_instance()->freeAllMemory();
			AtmiBrokerServiceFacMgr::discard_instance();
			//TODO READD AtmiBrokerNotify::discard_instance();
			AtmiBrokerOTS::discard_instance();
			AtmiBrokerEnv::discard_instance();
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleted services");

			shutdownBindings(server_orb, server_root_poa, server_root_poa_manager, server_default_context, server_name_context, server_poa, server_worker);
			return -1;
		}
		userlog(Level::getInfo(), loggerAtmiBrokerServer, (char*) "serverinit done");
		return 1;
	}
	return 0;
}

void serverdone() {
	try {
		_tperrno = 0;
		if (serverInitialized) {
			userlog(Level::getInfo(), loggerAtmiBrokerServer, (char*) "serverdone called ");

			// Ensure that the ORB is properly shutdown and cleaned up.
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone calling serverdone");
			tx_close();
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone called serverdone");

			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone shutting down services ");
			if (ptrServer) {
				ptrServer->serverdone();
			}

			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleting Server Factory ");
			if (ptrServerFactory) {
				delete ptrServerFactory;
			}
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleted Server Factory ");

			// TODO CLEAN UP TRANSACTION CURRENT
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleting services");
			AtmiBrokerMem::get_instance()->freeAllMemory();
			AtmiBrokerServiceFacMgr::discard_instance();
			//TODO READD AtmiBrokerNotify::discard_instance();
			AtmiBrokerOTS::discard_instance();
			AtmiBrokerEnv::discard_instance();
			userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleted services");

			shutdownBindings(server_orb, server_root_poa, server_root_poa_manager, server_default_context, server_name_context, server_poa, server_worker);

			userlog(Level::getInfo(), loggerAtmiBrokerServer, (char*) "serverdone returning");
			serverInitialized = false;
		}
	} catch (...) {
		userlog(Level::getError(), loggerAtmiBrokerServer, (char*) "main Unexpected exception in serverdone");
	}
}

// create_name_context -- create or find demo naming context.
// Raises a CORBA exception on error.
//
/*
 CosNaming::NamingContext_ptr
 create_name_context(
 CosNaming::NamingContextExt_ptr default_context,
 const char* name_string
 ) throw (
 CORBA::Exception
 )
 {
 CORBA::Object_var context_as_obj;

 // First see if the name is already bound.
 //
 try
 {
 context_as_obj = default_context->resolve_str(name_string);
 }
 catch (const CosNaming::NamingContext::NotFound&)
 {
 // Name is not yet bound, create a new context.
 //
 CosNaming::Name_var tmp_name =  default_context->to_name(name_string);
 context_as_obj = default_context->bind_new_context(tmp_name);
 }

 // Narrow the reference to a naming context.
 //
 CosNaming::NamingContext_ptr name_context = CosNaming::NamingContext::_narrow(context_as_obj);
 if (CORBA::is_nil(name_context)) {
 userlog(Level::getError(), loggerAtmiBrokerServer, (char*)  "create_name_context: Cannot narrow naming context called `%s'", name_string);
 throw CORBA::BAD_PARAM();
 }
 return name_context;
 }
 */

void createServerPOA() {
	if (CORBA::is_nil(server_poa)) {
		userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "createServerPOA creating POA %s", server);
		serverPoaFactory = new AtmiBrokerPoaFac();
		server_poa = serverPoaFactory->createServerPoa(server, server_root_poa, server_root_poa_manager);
		userlog(Level::getDebug(), loggerAtmiBrokerServer, (char*) "createServerPOA created Persistent POA: %p", (void*) server_poa);
	} else
		userlog(Level::getError(), loggerAtmiBrokerServer, (char*) "createServerPOA already created POA: %p", (void*) server_poa);
}

