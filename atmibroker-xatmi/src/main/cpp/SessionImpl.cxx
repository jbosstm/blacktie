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
#include "SenderImpl.h"
#include "ReceiverImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(CONNECTION* connection, int id, const char* serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	CosNaming::NamingContextExt_ptr context = (CosNaming::NamingContextExt_ptr) connection->default_ctx;
	CosNaming::NamingContext_ptr name_context = (CosNaming::NamingContext_ptr) connection->name_ctx;
	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << serviceName);
	CosNaming::Name * name = context->to_name(serviceName);
	CORBA::Object_var tmp_ref = name_context->resolve(*name);
	AtmiBroker::EndpointQueue_ptr remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "connected to " << serviceName);
	queueSender = new SenderImpl(remoteEndpoint);


	this->temporaryQueue = new EndpointQueue(connection);
	this->queueReceiver = new ReceiverImpl(temporaryQueue);
	this->replyTo = temporaryQueue->getName();
}

SessionImpl::SessionImpl(CONNECTION* connection, int id) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;

	queueSender = NULL;

	this->temporaryQueue = new EndpointQueue(connection);
	this->queueReceiver = new ReceiverImpl(temporaryQueue);
	this->replyTo = temporaryQueue->getName();
}

SessionImpl::~SessionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	if (queueReceiver) {
		delete queueReceiver;
		queueReceiver = NULL;
	}
	if (queueSender) {
		queueSender->close();
		delete queueSender;
		queueSender = NULL;
	}
	if (temporaryQueue) {
		delete temporaryQueue;
		temporaryQueue = NULL;
	}
}

void SessionImpl::setSendTo(char* destinationName) {
	if (queueSender) {
		delete queueSender;
		queueSender = NULL;
	}
	if (destinationName != NULL && strcmp(destinationName, "") != 0) {
		CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
		LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << destinationName);
		CORBA::Object_var tmp_ref = orb->string_to_object(destinationName);
		AtmiBroker::EndpointQueue_ptr remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
		LOG4CXX_DEBUG(logger, (char*) "connected to %s" << destinationName);

		queueSender = new SenderImpl(remoteEndpoint);
	}
}

Receiver * SessionImpl::getReceiver() {
	return queueReceiver;
}

Sender * SessionImpl::getSender() {
	return queueSender;
}

int SessionImpl::getId() {
	return id;
}

const char* SessionImpl::getReplyTo() {
	return replyTo;
}
