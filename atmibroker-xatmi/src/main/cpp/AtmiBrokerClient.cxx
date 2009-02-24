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

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

#include "AtmiBrokerClient.h"
#include "ConnectionImpl.h"
#include "xatmi.h"
#include "userlog.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerOTS.h"

AtmiBrokerClient * ptrAtmiBrokerClient;

log4cxx::LoggerPtr loggerAtmiBrokerClient(log4cxx::Logger::getLogger("AtmiBrokerClient"));

bool clientInitialized;

void client_sigint_handler_callback(int sig_type) {
	LOG4CXX_WARN(loggerAtmiBrokerClient, (char*) "client_sigint_handler_callback Received shutdown signal: " << sig_type);
	clientdone();
	abort();
}

int clientinit() {
	tperrno = 0;
	int toReturn = 0;

	initializeLogger();

	if (ptrAtmiBrokerClient == NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit called");
		signal(SIGINT, client_sigint_handler_callback);
		ptrAtmiBrokerClient = new AtmiBrokerClient();
		if (!clientInitialized) {
			::clientdone();
			toReturn = -1;
		} else {
			LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "Client Initialized");
		}
	}
	return toReturn;
}

int clientdone() {
	tperrno = 0;
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientdone called");
	if (ptrAtmiBrokerClient) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit deleting Corba Client");
		delete ptrAtmiBrokerClient;
		ptrAtmiBrokerClient = NULL;
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit deleted Corba Client");
	}
	return 0;
}

AtmiBrokerClient::AtmiBrokerClient() {
	try {

		clientConnection = new ConnectionImpl((char*) "client");

		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "constructor");

		AtmiBrokerClientXml aAtmiBrokerClientXml;
		aAtmiBrokerClientXml.parseXmlDescriptor(&clientServerVector, "CLIENT.xml");
		nextSessionId = 0;
		clientInitialized = true;
	} catch (...) {
		LOG4CXX_ERROR(loggerAtmiBrokerClient, (char*) "clientinit Unexpected exception");
		tperrno = TPESYSTEM;
	}
}

AtmiBrokerClient::~AtmiBrokerClient() {
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "destructor");

	for (std::vector<ClientServerInfo*>::iterator itServer = clientServerVector.begin(); itServer != clientServerVector.end(); itServer++) {
		LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "next serverName is: " << (char*) (*itServer)->serverName);
	}
	clientServerVector.clear();

	AtmiBrokerMem::discard_instance();
	//TODO READD AtmiBrokerNotify::discard_instance();
	AtmiBrokerOTS::discard_instance();
	AtmiBrokerEnv::discard_instance();
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "clientinit deleted services");

	if (clientConnection) {
		ConnectionImpl* conn = dynamic_cast<ConnectionImpl*> (clientConnection);
		delete conn;
		clientConnection = NULL;
	}
	clientInitialized = false;
	LOG4CXX_DEBUG(loggerAtmiBrokerClient, (char*) "Client Shutdown");

}

Session* AtmiBrokerClient::createSession(int& id, char* serviceName) {
	id = nextSessionId++;
	Session* session = clientConnection->createSession(id, serviceName);
	return session;
}

Session* AtmiBrokerClient::getSession(int id) {
	return clientConnection->getSession(id);
}

void AtmiBrokerClient::closeSession(int id) {
	clientConnection->closeSession(id);
}
