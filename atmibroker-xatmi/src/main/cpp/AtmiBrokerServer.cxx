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

#include "AtmiBrokerServer.h"
#include "OrbManagement.h"
#include "AtmiBroker.h"
#include "userlog.h"
#include "Connection.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBroker_ServerImpl.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerPoaFac.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

log4cxx::LoggerPtr loggerAtmiBrokerServer(log4cxx::Logger::getLogger("AtmiBrokerServer"));
AtmiBroker_ServerImpl * ptrServer;
bool serverInitialized;
CONNECTION* serverConnection;
PortableServer::POA_var server_poa;

void server_sigint_handler_callback(int sig_type) {
	userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "server_sigint_handler_callback Received shutdown signal: %d", sig_type);
	serverdone();
	abort();
}

int serverrun() {
	int toReturn = 0;
	try {
		userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, "Server waiting for requests...");
		((CORBA::ORB_ptr) serverConnection->orbRef)->run();
	} catch (CORBA::Exception& e) {
		userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, "Unexpected CORBA exception: %s", e._name());
		toReturn = -1;
	}
	return toReturn;
}

int serverinit(int argc, char ** argv) {
	_tperrno = 0;
	int toReturn = -1;
	if (!loggerInitialized) {
		if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
			log4cxx::PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
		} else {
			log4cxx::BasicConfigurator::configure();
		}
		loggerInitialized = true;
	}

	if (!serverInitialized) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit called");
		signal(SIGINT, server_sigint_handler_callback);
		try {
			serverConnection = AtmiBrokerOTS::init_orb((char*) "server");
			userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "creating POAs for %s", server);
			AtmiBrokerPoaFac* serverPoaFactory = (AtmiBrokerPoaFac*) serverConnection->poaFactory;
			server_poa = serverPoaFactory->createServerPoa(((CORBA::ORB_ptr) serverConnection->orbRef), server, ((PortableServer::POA_ptr) serverConnection->root_poa), ((PortableServer::POAManager_ptr) serverConnection->root_poa_manager));
			ptrServer = new AtmiBroker_ServerImpl(serverConnection, server_poa);
			ptrServer->server_init();
			serverInitialized = true;
			toReturn = 0;
			userlog(log4cxx::Level::getInfo(), loggerAtmiBrokerServer, (char*) "Server Running");
		} catch (CORBA::Exception& e) {
			userlog(log4cxx::Level::getError(), loggerAtmiBrokerServer, (char*) "serverinit - Unexpected CORBA exception: %s", e._name());
			tperrno = TPESYSTEM;
			::serverdone();
		}
	}
	return toReturn;
}

int serverdone() {
	_tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone called ");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverdone shutting down services ");
	if (ptrServer) {
		ptrServer->server_done();
		delete ptrServer;
		ptrServer = NULL;
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleting services");
	AtmiBrokerMem::discard_instance();
	//TODO READD AtmiBrokerNotify::discard_instance();
	AtmiBrokerOTS::discard_instance();
	AtmiBrokerEnv::discard_instance();
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerServer, (char*) "serverinit deleted services");

	if (serverConnection) {
		shutdownBindings(serverConnection);
		serverConnection = NULL;
	}
	serverInitialized = false;
	return 0;
}
