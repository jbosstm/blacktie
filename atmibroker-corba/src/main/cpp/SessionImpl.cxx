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
#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include <string.h>
#include "SessionImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(ConnectionImpl* connection, int id, const char* serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << serviceName);
	CosNaming::NamingContextExt_ptr context = connection->getRealConnection()->default_ctx;
	CosNaming::NamingContext_ptr name_context = connection->getRealConnection()->name_ctx;
	CosNaming::Name * name = context->to_name(serviceName);
	CORBA::Object_var tmp_ref = name_context->resolve(*name);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	this->sendTo = (char*) serviceName;
	LOG4CXX_DEBUG(logger, (char*) "connected to " << serviceName);

	this->temporaryQueue = new EndpointQueue(connection->getRealConnection());
	this->replyTo = temporaryQueue->getName();

	this->canSend = true;
	this->canRecv = true;
}

SessionImpl::SessionImpl(ConnectionImpl* connection, int id) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	remoteEndpoint = NULL;

	this->temporaryQueue = new EndpointQueue(connection->getRealConnection());
	this->replyTo = temporaryQueue->getName();

	this->canSend = true;
	this->canRecv = true;
}

SessionImpl::~SessionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	if (remoteEndpoint) {
		LOG4CXX_DEBUG(logger, (char*) "disconnecting from: " << sendTo);
		remoteEndpoint->disconnect();
		LOG4CXX_DEBUG(logger, (char*) "disconnected from: " << sendTo);
		remoteEndpoint = NULL;
	}
	LOG4CXX_DEBUG(logger, (char*) "destructed");
	//	if (temporaryQueue) {
	//		delete temporaryQueue;
	//		temporaryQueue = NULL;
	//	}
}

void SessionImpl::setSendTo(char* destinationName) {
	if (remoteEndpoint) {
		remoteEndpoint = NULL;
	}
	if (destinationName != NULL && strcmp(destinationName, "") != 0) {
		CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->getRealConnection()->orbRef;
		LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << destinationName);
		CORBA::Object_var tmp_ref = orb->string_to_object(destinationName);
		remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
		LOG4CXX_DEBUG(logger, (char*) "connected to %s" << destinationName);
	}
	this->sendTo = destinationName;
}

char* SessionImpl::getSendTo() {
	return this->sendTo;
}

MESSAGE SessionImpl::receive(long time) {
	LOG4CXX_DEBUG(logger, (char*) "Receiving from: " << replyTo);
	return temporaryQueue->receive(time);
}

void SessionImpl::send(MESSAGE message) {
	unsigned char * data_togo = (unsigned char *) malloc(message.len);
	memcpy(data_togo, message.data, message.len);
	AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(message.len, message.len, data_togo, true);
	remoteEndpoint->send(message.replyto, message.rval, message.rcode, aOctetSeq, message.len, message.correlationId, message.flags);
	aOctetSeq = NULL;
	LOG4CXX_DEBUG(logger, (char*) "Called back ");
}

int SessionImpl::getId() {
	return id;
}

const char* SessionImpl::getReplyTo() {
	return replyTo;
}

void SessionImpl::setCanSend(bool canSend) {
	this->canSend = canSend;
}

void SessionImpl::setCanRecv(bool canRecv) {
	this->canRecv = canRecv;
}

bool SessionImpl::getCanSend() {
	return canSend;
}

bool SessionImpl::getCanRecv() {
	return canRecv;
}

Destination* SessionImpl::getDestination() {
	return temporaryQueue;
}
