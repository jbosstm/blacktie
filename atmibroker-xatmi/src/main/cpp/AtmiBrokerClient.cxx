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


AtmiBrokerClient * ptrAtmiBrokerClient;

log4cxx::LoggerPtr loggerAtmiBrokerClient(log4cxx::Logger::getLogger(
		"AtmiBrokerClient"));

bool clientInitialized;

void client_sigint_handler_callback(int sig_type) {
	signal(SIGINT, SIG_IGN);
	LOG4CXX_WARN(loggerAtmiBrokerClient,
			(char*) "SIGINT Detected: Shutting down client this may take several minutes");
	clientdone();
	LOG4CXX_WARN(loggerAtmiBrokerClient,
			(char*) "Shutdown complete");
	abort();
}

int clientinit() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = 0;

	initializeLogger();

	if (ptrAtmiBrokerClient == NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit called");
		ptrAtmiBrokerClient = new AtmiBrokerClient();
		if (!clientInitialized) {
			::clientdone();
			toReturn = -1;
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		} else {
			//signal(SIGINT, client_sigint_handler_callback);
			ACE_Sig_Action sa (
					reinterpret_cast <ACE_SignalHandler> (
						client_sigint_handler_callback));

  			// Make sure we specify that SIGINT will be masked out
  			// during the signal handler's execution.
  			ACE_Sig_Set ss;
  			ss.sig_add (SIGINT);
  			sa.mask (ss);
  			sa.register_action (SIGINT);

			LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "Client Initialized");
		}
	}
	return toReturn;
}

int clientdone() {
	setSpecific(TPE_KEY, TSS_TPERESET);
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientdone called");
	if (ptrAtmiBrokerClient) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient,
				(char*) "clientinit deleting Corba Client");
		delete ptrAtmiBrokerClient;
		ptrAtmiBrokerClient = NULL;
		LOG4CXX_DEBUG(loggerAtmiBrokerClient,
				(char*) "clientinit deleted Corba Client");
	}
	return 0;
}

AtmiBrokerClient::AtmiBrokerClient() {
	try {
		nextSessionId = 0;
		clientInitialized = true;
		currentConnection = NULL;

	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerClient,
				(char*) "clientinit Unexpected exception");
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}
}

AtmiBrokerClient::~AtmiBrokerClient() {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "destructor");
	AtmiBrokerMem::discard_instance();
	txx_stop();
	AtmiBrokerEnv::discard_instance();
	clientConnectionManager.closeConnections();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit deleted services");
	clientInitialized = false;
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "Client Shutdown");
}

Session* AtmiBrokerClient::createSession(int& id, char* serviceName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "creating session: " << serviceName);
	char* svc;
	char  adm[16];

	svc = serviceName;
	if(strstr(serviceName, "ADMIN") != NULL) {
		int i;
		for(i = strlen(serviceName); i >=0 && serviceName[i] != '_'; i--);
		if(i > 0) {
			memset(adm, 0, 16);
			ACE_OS::strncpy(adm, serviceName, i);
			svc = adm;
		}
	}

	Connection* clientConnection = clientConnectionManager.getClientConnection(svc);

	if(clientConnection != NULL) {
		currentConnection = clientConnection;
		id = nextSessionId++;
		Session* session = clientConnection->createSession(id, serviceName);
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "created session: " << id << " send: " << session->getCanSend() << " recv: " << session->getCanRecv());
		return session;
	}

	return NULL;
}

Session* AtmiBrokerClient::getSession(int id) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "get session: " << id);
	Session* session = NULL;
	if (currentConnection != NULL) {
		session = currentConnection->getSession(id);
	}
	if (session != NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "got session: " << id << " send: " << session->getCanSend() << " recv: " << session->getCanRecv());
	} else {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "did not get session: " << id);
	}
	return session;
}

void AtmiBrokerClient::closeSession(int id) {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "close session: " << id);
	if(currentConnection != NULL) {
		currentConnection->closeSession(id);
	}
}
