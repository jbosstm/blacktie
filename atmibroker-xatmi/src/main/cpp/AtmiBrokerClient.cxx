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

#include "AtmiBrokerInit.h"
#include "AtmiBrokerClient.h"
#include "xatmi.h"
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
	LOG4CXX_INFO(
			loggerAtmiBrokerClient,
			(char*) "SIGINT Detected: Shutting down client this may take several minutes");
	clientdone(sig_type);
	LOG4CXX_INFO(loggerAtmiBrokerClient, (char*) "Shutdown complete");
	return 0;
}

int clientinit() {
	AtmiBrokerInitSingleton::instance();
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;

	client_lock.lock();
	if (ptrAtmiBrokerClient == NULL) {
		try {
			// This must be in the try catch as the configuration may not exist
			AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
			LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit called");
			ptrAtmiBrokerClient = new AtmiBrokerClient(env->getSignalHandler());
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
				(env->getSignalHandler()).addSignalHandler(
						client_sigint_handler_callback);

				LOG4CXX_DEBUG(loggerAtmiBrokerClient,
						(char*) "Client Initialized");
				toReturn = 0;
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerAtmiBrokerClient, (char*) "clientinit failed");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}

	} else {
		toReturn = 0;
	}
	client_lock.unlock();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit returning "
			<< toReturn);
	return toReturn;
}

int clientdone(int reason = 0) {
	setSpecific(TPE_KEY, TSS_TPERESET);
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientdone called");

	client_lock.lock();
	if (ptrAtmiBrokerClient) {
		if (reason == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerClient,
					(char*) "clientdone deleting Corba Client");
			delete ptrAtmiBrokerClient;
			ptrAtmiBrokerClient = NULL;
			LOG4CXX_DEBUG(loggerAtmiBrokerClient,
					(char*) "clientdone deleted Corba Client");
		} else {
			// cannot use closeSession since it deletes the underlying queue - instead
			// we need to interrupt any threads waiting on the underlying queues:
			LOG4CXX_DEBUG(loggerAtmiBrokerClient,
					(char*) "clientdone interrupting connections");
			ptrAtmiBrokerClient->disconnectSessions();
			LOG4CXX_DEBUG(loggerAtmiBrokerClient,
					(char*) "clientdone connections interrupted");
		}
	}
	client_lock.unlock();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientdone returning 0");
	return 0;
}

AtmiBrokerClient::AtmiBrokerClient(AtmiBrokerSignalHandler& handler) :
	currentConnection(NULL), signalHandler(handler) {
	try {
		lock = new SynchronizableObject();
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, "Created lock: " << lock);
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

Session* AtmiBrokerClient::createSession(bool isConv, int& id,
		char* serviceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "creating session: "
			<< serviceName);
	if (serviceName == NULL) {
		setSpecific(TPE_KEY, TSS_TPEINVAL);
		return NULL;
	}
	Session* session = NULL;

	Connection* clientConnection = NULL;
	clientConnection = clientConnectionManager.getClientConnection();

	if (clientConnection != NULL) {
		lock->lock();
		currentConnection = clientConnection;
		lock->unlock();

		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "created session: " << id);
		session = clientConnection->createSession(isConv, serviceName);

		if (session == NULL) {
			LOG4CXX_ERROR(loggerAtmiBrokerClient,
					(char*) "null session created: " << id);
		}
		id = session->getId();
		session->setSigHandler(&(signalHandler));
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "created session: " << id
				<< " send: " << session->getCanSend() << " recv: "
				<< session->getCanRecv());
	}

	return session;
}

Session* AtmiBrokerClient::getQueueSession() {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "get queue session");
	Session* session = NULL;
	if (currentConnection != NULL) {
		session = currentConnection->getQueueSession();
	}
	if (session != NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "got queue session"
				<< " send: " << session->getCanSend() << " recv: "
				<< session->getCanRecv());
	} else {
		LOG4CXX_ERROR(loggerAtmiBrokerClient,
				(char*) "did not get queue session");
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

void AtmiBrokerClient::disconnectSessions() {
	if (currentConnection != NULL) {
		currentConnection->disconnectSession(-1);
	} else {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient,
				(char*) "AtmiBrokerClient no connections to disconnect");
	}
}
