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
#include "SessionImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(CORBA_CONNECTION* connection, int id, char* serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	this->sendTo = (char*) ::malloc(XATMI_SERVICE_NAME_LENGTH + 1);
	memset(this->sendTo, '\0', XATMI_SERVICE_NAME_LENGTH + 1);
	strncat(this->sendTo, serviceName, XATMI_SERVICE_NAME_LENGTH);

	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << sendTo);
	CosNaming::NamingContextExt_ptr context = connection->default_ctx;
	CosNaming::NamingContext_ptr name_context = connection->name_ctx;
	CosNaming::Name * name = context->to_name(this->sendTo);
	CORBA::Object_var tmp_ref = name_context->resolve(*name);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "connected to " << sendTo);

	this->temporaryQueue = new EndpointQueue(connection);
	this->replyTo = temporaryQueue->getName();

	this->canSend = true;
	this->canRecv = true;
}

SessionImpl::SessionImpl(CORBA_CONNECTION* connection, int id, const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << temporaryQueueName);
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	CORBA::Object_var tmp_ref = orb->string_to_object(temporaryQueueName);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "connected to %s" << temporaryQueueName);

	this->temporaryQueue = new EndpointQueue(connection);
	this->replyTo = temporaryQueue->getName();

	this->canSend = true;
	this->canRecv = true;
}

SessionImpl::~SessionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	if (remoteEndpoint) {
		LOG4CXX_DEBUG(logger, (char*) "disconnecting from: remote endpoint");
		remoteEndpoint->disconnect();
		LOG4CXX_DEBUG(logger, (char*) "disconnected from: remote endpoint");
		remoteEndpoint = NULL;
	}
	LOG4CXX_DEBUG(logger, (char*) "destructed");
	//	if (temporaryQueue) {
	//		delete temporaryQueue;
	//		temporaryQueue = NULL;
	//	}
}

void SessionImpl::setSendTo(const char* destinationName) {
	if (remoteEndpoint) {
		remoteEndpoint = NULL;
	}
	if (destinationName != NULL && strcmp(destinationName, "") != 0) {
		CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
		LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << destinationName);
		CORBA::Object_var tmp_ref = orb->string_to_object(destinationName);
		remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
		LOG4CXX_DEBUG(logger, (char*) "connected to %s" << destinationName);
	}
	this->sendTo = (char*) destinationName;
}

MESSAGE SessionImpl::receive(long time) {
	LOG4CXX_DEBUG(logger, (char*) "Receiving from: " << replyTo);
	return temporaryQueue->receive(time);
}

bool SessionImpl::send(MESSAGE message) {
	AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(message.len, message.len, (unsigned char*) message.data, true);
	remoteEndpoint->send(message.replyto, message.rval, message.rcode, aOctetSeq, message.len, message.correlationId, message.flags);
	aOctetSeq = NULL;
	LOG4CXX_DEBUG(logger, (char*) "Called back ");
	return true;
}

int SessionImpl::getId() {
	return id;
}

const char* SessionImpl::getReplyTo() {
	return replyTo;
}
