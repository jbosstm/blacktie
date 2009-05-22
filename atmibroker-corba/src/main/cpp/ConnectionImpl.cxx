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
#include "OrbManagement.h"
#include "txClient.h"
#include "AtmiBrokerPoaFac.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr CorbaConnectionImpl::logger(log4cxx::Logger::getLogger(
		"CorbaConnectionImpl"));

CorbaConnectionImpl::CorbaConnectionImpl(char* connectionName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor");
	this->connection = (CORBA_CONNECTION *) start_tx_orb(connectionName);
}

CorbaConnectionImpl::~CorbaConnectionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	shutdownBindings(this->connection);
}

Session* CorbaConnectionImpl::createSession(int id, char * serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession: " + id);
	sessionMap[id] = new CorbaSessionImpl(this->connection, id, serviceName);
	return sessionMap[id];
}

Session* CorbaConnectionImpl::createSession(int id, const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	return new CorbaSessionImpl(this->connection, id, temporaryQueueName);
}

Session* CorbaConnectionImpl::getSession(int id) {
	return sessionMap[id];
}

void CorbaConnectionImpl::closeSession(int id) {
	if (sessionMap[id]) {
		delete sessionMap[id];
		sessionMap[id] = NULL;
	}
}

Destination* CorbaConnectionImpl::createDestination(char* serviceName) {
	// create Poa for Service Queue
	AtmiBrokerPoaFac* poaFactory = this->connection->poaFactory;
	PortableServer::POA_ptr aFactoryPoaPtr = poaFactory->createServicePoa(
			this->connection->orbRef, serviceName, this->connection->root_poa,
			this->connection->root_poa_manager);
	LOG4CXX_DEBUG(logger, (char*) "created create_service_factory_poa: "
			<< aFactoryPoaPtr);

	return new CorbaEndpointQueue(this->connection, aFactoryPoaPtr, serviceName);
}

void CorbaConnectionImpl::destroyDestination(Destination* destination) {
	char* serviceName = strdup(destination->getName());
	CosNaming::Name * name = this->connection->default_ctx->to_name(
			destination->getName());
	LOG4CXX_DEBUG(logger, (char*) "unbinding: " << serviceName);
	this->connection->name_ctx->unbind(*name);
	LOG4CXX_DEBUG(logger, (char*) "unbound: " << serviceName);

	CorbaEndpointQueue* queue = dynamic_cast<CorbaEndpointQueue*> (destination);
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
	free(serviceName);
}

