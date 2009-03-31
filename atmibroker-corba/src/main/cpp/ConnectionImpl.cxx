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
#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include <string.h>
#include "ConnectionImpl.h"
#include "SessionImpl.h"
#include "txClient.h"
#include "OrbManagement.h"
#include "AtmiBrokerPoaFac.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr ConnectionImpl::logger(log4cxx::Logger::getLogger(
		"ConnectionImpl"));

ConnectionImpl::ConnectionImpl(char* connectionName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor");
	this->connection = startTxOrb(connectionName);
}

ConnectionImpl::~ConnectionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	shutdownBindings(this->connection);
}

Session* ConnectionImpl::createSession(int id, char * serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	sessionMap[id] = new SessionImpl(this->connection, id, serviceName);
	return sessionMap[id];
}

Session* ConnectionImpl::createSession(int id, const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	return new SessionImpl(this->connection, id, temporaryQueueName);
}

Session* ConnectionImpl::getSession(int id) {
	return sessionMap[id];
}

void ConnectionImpl::closeSession(int id) {
	if (sessionMap[id]) {
		delete sessionMap[id];
		sessionMap[id] = NULL;
	}
}

Destination* ConnectionImpl::createDestination(char* serviceName) {
	// create Poa for Service Queue
	AtmiBrokerPoaFac* poaFactory = this->connection->poaFactory;
	PortableServer::POA_ptr aFactoryPoaPtr = poaFactory->createServicePoa(
			this->connection->orbRef, serviceName, this->connection->root_poa,
			this->connection->root_poa_manager);
	LOG4CXX_DEBUG(logger, (char*) "created create_service_factory_poa: "
			<< aFactoryPoaPtr);

	return new EndpointQueue(this->connection, aFactoryPoaPtr, serviceName);
}

void ConnectionImpl::destroyDestination(Destination* destination) {
	char* serviceName = strdup(destination->getName());
	CosNaming::Name * name = this->connection->default_ctx->to_name(
			destination->getName());
	LOG4CXX_DEBUG(logger, (char*) "unbinding: " << serviceName);
	this->connection->name_ctx->unbind(*name);
	LOG4CXX_DEBUG(logger, (char*) "unbound: " << serviceName);

	EndpointQueue* queue = dynamic_cast<EndpointQueue*> (destination);
	/*
	PortableServer::POA_ptr poa = queue->getPoa();
	*/
	LOG4CXX_DEBUG(logger, (char*) "deleting queue: " << queue);
	delete queue;
	/*
	LOG4CXX_DEBUG(logger, (char*) "destroying poa: " << poa);
	try {
		poa->destroy(true, true);
	} catch (...) {
		LOG4CXX_DEBUG(logger, (char*) "could not destroy poa: " << poa);
	}
	LOG4CXX_DEBUG(logger, (char*) "destroyed poa: " << poa);
	poa = NULL;
	*/
	LOG4CXX_DEBUG(logger, (char*) "destination destroyed: " << serviceName);
}

