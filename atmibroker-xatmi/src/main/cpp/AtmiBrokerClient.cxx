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

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

#include "AtmiBrokerClient.h"
#include "xatmi.h"
#include "userlog.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"
#include "txx.h"
#include "SymbolLoader.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"
#include "ace/Signal.h"
#include "ThreadLocalStorage.h"
#include "AtmiBrokerSignalHandler.h"

AtmiBrokerClient * ptrAtmiBrokerClient;

log4cxx::LoggerPtr loggerAtmiBrokerClient(log4cxx::Logger::getLogger(
		"AtmiBrokerClient"));

bool clientInitialized = false;
SynchronizableObject client_lock;

int client_sigint_handler_callback(int sig_type) {
	LOG4CXX_WARN(
			loggerAtmiBrokerClient,
			(char*) "SIGINT Detected: Shutting down client this may take several minutes");
	clientdone();
	LOG4CXX_WARN(loggerAtmiBrokerClient, (char*) "Shutdown complete");
	exit(1);
	/* NOTREACHED*/
	return -1;
}

int clientinit() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;

	client_lock.lock();
	if (ptrAtmiBrokerClient == NULL) {
		try {
			AtmiBrokerEnv::get_instance();
			LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit called");
			ptrAtmiBrokerClient = new AtmiBrokerClient();
			if (!clientInitialized) {
				LOG4CXX_DEBUG(loggerAtmiBrokerClient,
						(char*) "clientinit deleting Client");
				delete ptrAtmiBrokerClient;
				ptrAtmiBrokerClient = NULL;
				LOG4CXX_DEBUG(loggerAtmiBrokerClient,
						(char*) "clientinit deleted Client");
				setSpecific(TPE_KEY, TSS_TPESYSTEM);
			} else {
				// install a handler for SIGINT and SIGTERM
				(ptrAtmiBrokerClient->getSigHandler()).setSigHandler(client_sigint_handler_callback);

				LOG4CXX_DEBUG(loggerAtmiBrokerClient,
						(char*) "Client Initialized");
				toReturn = 0;
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerClient,
					(char*) "clientinit Unexpected exception");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}
	} else {
		toReturn = 0;
	}
	client_lock.unlock();
	return toReturn;
}

int clientdone() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientdone called");

	client_lock.lock();
	if (ptrAtmiBrokerClient) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient,
				(char*) "clientdone deleting Corba Client");
		delete ptrAtmiBrokerClient;
		ptrAtmiBrokerClient = NULL;
		LOG4CXX_DEBUG(loggerAtmiBrokerClient,
				(char*) "clientdone deleted Corba Client");
	}
	client_lock.unlock();

	return 0;
}

AtmiBrokerClient::AtmiBrokerClient() : currentConnection(NULL), nextSessionId(0) {
	try {
		lock = new SynchronizableObject();
		clientInitialized = true;
	} catch (...) {
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
		LOG4CXX_ERROR(loggerAtmiBrokerClient,
				(char*) "clientinit Unexpected exception");
	}
}

AtmiBrokerClient::~AtmiBrokerClient() {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "destructor");
	AtmiBrokerMem::discard_instance();
	txx_stop();
	clientConnectionManager.closeConnections();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit deleted services");
	delete lock;
	clientInitialized = false;
	AtmiBrokerEnv::discard_instance();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "Client Shutdown");
}

Session* AtmiBrokerClient::createSession(int& id, char* serviceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "creating session: "
			<< serviceName);
	if (serviceName == NULL) {
		setSpecific(TPE_KEY, TSS_TPEINVAL);
		return NULL;
	}
	Session* session = NULL;

	Connection* clientConnection = NULL;
	clientConnection = clientConnectionManager.getClientConnection(serviceName);

	if (clientConnection != NULL) {
		lock->lock();
		currentConnection = clientConnection;
		id = nextSessionId++;
		lock->unlock();

		session = clientConnection->createSession(id, serviceName);
		session->setSigHandler(&sigHandler);
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "created session: " << id
				<< " send: " << session->getCanSend() << " recv: "
				<< session->getCanRecv());
	}

	return session;
}

Session* AtmiBrokerClient::getSession(int id) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "get session: " << id);
	Session* session = NULL;
	if (currentConnection != NULL) {
		session = currentConnection->getSession(id);
	}
	if (session != NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "got session: " << id
				<< " send: " << session->getCanSend() << " recv: "
				<< session->getCanRecv());
	} else {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "did not get session: "
				<< id);
	}
	return session;
}

void AtmiBrokerClient::closeSession(int id) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "close session: " << id);
	if (currentConnection != NULL) {
		currentConnection->closeSession(id);
	}
}
