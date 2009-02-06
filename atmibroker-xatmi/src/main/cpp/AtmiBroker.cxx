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
#include "AtmiBroker.h"
#include "xatmi.h"
#include "OrbManagement.h"
#include "userlog.h"
#include "Connection.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerOTS.h"
#include "EndpointQueue.h"
#include "SenderImpl.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

// Global state
log4cxx::LoggerPtr loggerAtmiBroker(log4cxx::Logger::getLogger("AtmiBroker"));
AtmiBrokerClient * ptrAtmiBrokerClient;
bool loggerInitialized;
bool clientInitialized;
CONNECTION* clientConnection;

void client_sigint_handler_callback(int sig_type) {
	userlog(log4cxx::Level::getWarn(), loggerAtmiBroker, (char*) "client_sigint_handler_callback Received shutdown signal: %d", sig_type);
	clientdone();
	abort();
}

int clientinit() {
	_tperrno = 0;
	int toReturn = 0;
	if (!loggerInitialized) {
		if (AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG") != NULL) {
			log4cxx::PropertyConfigurator::configure(AtmiBrokerEnv::get_instance()->getenv((char*) "LOG4CXXCONFIG"));
		} else {
			log4cxx::BasicConfigurator::configure();
		}
		loggerInitialized = true;
	}

	if (!clientInitialized) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit called");
		signal(SIGINT, client_sigint_handler_callback);
		try {
			clientConnection = AtmiBrokerOTS::init_orb((char*) "client");
			ptrAtmiBrokerClient = new AtmiBrokerClient();
			clientInitialized = true;
			userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "Client Initialized");
		} catch (CORBA::Exception &ex) {
			userlog(log4cxx::Level::getError(), loggerAtmiBroker, (char*) "clientinit Unexpected CORBA exception: %s", ex._name());
			_tperrno = TPESYSTEM;
			::clientdone();
			toReturn = -1;
		}
	}
	return toReturn;
}

int clientdone() {
	_tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientdone called");

	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleting services");
	if (ptrAtmiBrokerClient) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleting Corba Client ");
		delete ptrAtmiBrokerClient;
		ptrAtmiBrokerClient = NULL;
		userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleted Corba Client ");
	}
	AtmiBrokerMem::discard_instance();
	//TODO READD AtmiBrokerNotify::discard_instance();
	AtmiBrokerOTS::discard_instance();
	AtmiBrokerEnv::discard_instance();
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "clientinit deleted services");

	if (clientConnection) {
		// DISCARD THE CONNECTION
		shutdownBindings(clientConnection);
		clientConnection = NULL;
	}
	clientInitialized = false;
	userlog(log4cxx::Level::getDebug(), loggerAtmiBroker, (char*) "Client Shutdown");
	return 0;
}

Sender* get_service_queue_sender(const char * serviceName) {
	return new SenderImpl(new EndpointQueue(clientConnection, serviceName));
}
