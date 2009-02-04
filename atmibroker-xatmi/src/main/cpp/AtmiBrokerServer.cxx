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

#include "AtmiBroker.h"
#include "AtmiBrokerServiceFacMgr.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBroker_ServerImpl.h"
//TODO READD #include "AtmiBrokerNotify.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger("AtmiBrokerServer"));

bool serverInitialized;

AtmiBroker_ServerImpl * ptrServer;

Worker* server_worker;
CORBA::ORB_var server_orb;
PortableServer::POA_var server_root_poa;
PortableServer::POAManager_var server_root_poa_manager;
CosNaming::NamingContextExt_var server_default_context;
CosNaming::NamingContext_var server_name_context;
PortableServer::POA_var server_poa;
PortableServer::POA_var server_callback_poa;
AtmiBrokerPoaFac * serverPoaFactory;

void server_sigint_handler_callback(int sig_type) {
	userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "server_sigint_handler_callback Received shutdown signal: %d", sig_type);
	serverdone();
	abort();
}

int serverrun() {
	int toReturn = 0;
	try {
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, "Server waiting for requests...");
		server_orb->run();
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, "Unexpected CORBA exception: %s", e._name());
		toReturn = -1;
	}
	return toReturn;
}

int serverinit(int argc, char ** argv) {
	if (!serverInitialized) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit called");
		_tperrno = 0;
		signal(SIGINT, server_sigint_handler_callback);

		if (!loggerInitialized) {
			if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
				log4cxx::PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
			} else {
				log4cxx::BasicConfigurator::configure();
			}
			loggerInitialized = true;
		}

		try {
			AtmiBrokerOTS::init_orb((char*) "server", server_worker, server_orb, server_default_context, server_name_context);

			AtmiBrokerMem::get_instance();
			getRootPOAAndManager(server_orb, server_root_poa, server_root_poa_manager);

			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "creating POAs for %s", server);
			serverPoaFactory = new AtmiBrokerPoaFac();
			server_poa = serverPoaFactory->createServerPoa(server_orb, server, server_root_poa, server_root_poa_manager);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "created server POA: %p", (void*) server_poa);
			std::string name = ".server";
			server_callback_poa = serverPoaFactory->createCallbackPoa(server_orb, name.c_str(), server_root_poa, server_root_poa_manager);
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "created callback POA: %p", (void*) server_callback_poa);

			server_root_poa_manager->activate();
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "activated poa - started processing requests ");

			// Create a reference for interface AtmiBroker::Server.
			ptrServer = new AtmiBroker_ServerImpl(server_poa);
			if (ptrServer->server_init() != -1) {
				serverInitialized = true;
			}
		} catch (CORBA::Exception& e) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, (char*) "serverinit - Unexpected CORBA exception: %s", e._name());
		} catch (...) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, (char*) "serverinit - Unexpected exception");
		}

		if (!serverInitialized) {
			tperrno = TPESYSTEM;

			// CLEAN UP INITIALISED ITEMS
			if (ptrServer) {
				ptrServer->server_done();
				delete ptrServer;
				ptrServer = NULL;
			}

			// TODO CLEAN UP TRANSACTION CURRENT
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleting services");
			AtmiBrokerMem::discard_instance();
			AtmiBrokerServiceFacMgr::discard_instance();
			//TODO READD AtmiBrokerNotify::discard_instance();
			AtmiBrokerOTS::discard_instance();
			AtmiBrokerEnv::discard_instance();
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleted services");

			shutdownBindings(server_orb, server_root_poa, server_root_poa_manager, server_default_context, server_name_context, server_poa, server_worker);
			return -1;
		}
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "Server Running");
		return 1;
	}
	return 0;
}

int serverdone() {
	try {
		_tperrno = 0;
		if (serverInitialized) {
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone called ");

			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone shutting down services ");
			if (ptrServer) {
				ptrServer->server_done();
				delete ptrServer;
				ptrServer = NULL;
			}

			// TODO CLEAN UP TRANSACTION CURRENT
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleting services");
			AtmiBrokerMem::discard_instance();
			AtmiBrokerServiceFacMgr::discard_instance();
			//TODO READD AtmiBrokerNotify::discard_instance();
			AtmiBrokerOTS::discard_instance();
			AtmiBrokerEnv::discard_instance();
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone deleted services");

			shutdownBindings(server_orb, server_root_poa, server_root_poa_manager, server_default_context, server_name_context, server_poa, server_worker);

			userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "Server shutdown");
			serverInitialized = false;
		}
		return 0;
	} catch (...) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, (char*) "main Unexpected exception in serverdone");
		return -1;
	}
}
