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

#include <string.h>
#include <exception>

#include "malloc.h"
#include "SessionImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(stomp_connection* connection, apr_pool_t* pool, int id, char* serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;
	this->pool = pool;
	this->canSend = true;
	this->canRecv = true;
	this->sendTo = (char*) ::malloc(8 + 16);
	strcat(this->sendTo, "/queue/");
	strncat(this->sendTo, serviceName, 16);

	this->toRead = new EndpointQueue(this->connection, this->pool, id);
	LOG4CXX_DEBUG(logger, "OK");
}

SessionImpl::SessionImpl(stomp_connection* connection, apr_pool_t* pool, int id, const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor ");
	this->id = id;
	this->connection = connection;
	this->pool = pool;
	this->canSend = true;
	this->canRecv = true;
	this->sendTo = ::strdup(temporaryQueueName);

	this->toRead = new EndpointQueue(this->connection, this->pool, id);
	LOG4CXX_DEBUG(logger, "OK");
}

SessionImpl::~SessionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destroyed");
}

MESSAGE SessionImpl::receive(long time) {
	return toRead->receive(time);
}

void SessionImpl::send(MESSAGE message) {
	stomp_frame frame;
	frame.command = (char*) "SEND";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, sendTo);

	frame.body_length = message.len;
	frame.body = message.data;
//	apr_hash_set(frame.headers, "replyto", APR_HASH_KEY_STRING, message.replyto);
//	apr_hash_set(frame.headers, "correlationId", APR_HASH_KEY_STRING, message.correlationId);
//	apr_hash_set(frame.headers, "message.flags", APR_HASH_KEY_STRING, message.flags);
//	apr_hash_set(frame.headers, "message.control", APR_HASH_KEY_STRING, message.control);
//	apr_hash_set(frame.headers, "message.rval", APR_HASH_KEY_STRING, message.rval);
//	apr_hash_set(frame.headers, "message.rcode", APR_HASH_KEY_STRING, message.rcode);
//	apr_hash_set(frame.headers, "message.event", APR_HASH_KEY_STRING, message.event);

	LOG4CXX_DEBUG(logger, "Sending SEND");
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not send frame");
		throw std::exception();
	}
	LOG4CXX_DEBUG(logger, (char*) "Called back ");
}

void SessionImpl::setSendTo(const char* destinationName) {
	this->sendTo = (char*) destinationName;
}

const char* SessionImpl::getReplyTo() {
	return this->toRead->getName();
}

int SessionImpl::getId() {
	return id;
}
