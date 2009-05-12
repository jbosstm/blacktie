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
#include <exception>

#include "apr_strings.h"
#include "malloc.h"
#include "EndpointQueue.h"
#include "ThreadLocalStorage.h"
#include "ConnectionImpl.h"

log4cxx::LoggerPtr EndpointQueue::logger(log4cxx::Logger::getLogger(
		"EndpointQueue"));

EndpointQueue::EndpointQueue(apr_pool_t* pool,
		char* serviceName) {
	shutdown = false;
	lock = new SynchronizableObject();
	
	connection = ConnectionImpl::connect(pool, 2); // TODO allow the timeout to be specified in configuration
	this->pool = pool;

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	char* queueName = (char*) ::malloc(8 + XATMI_SERVICE_NAME_LENGTH + 1);
	memset(queueName, '\0', 8 + XATMI_SERVICE_NAME_LENGTH + 1);
	strcpy(queueName, "/queue/");
	strncat(queueName, serviceName, XATMI_SERVICE_NAME_LENGTH);

	stomp_frame frame;
	frame.command = (char*) "SUB";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, queueName);
	frame.body_length = -1;
	frame.body = NULL;
	LOG4CXX_DEBUG(logger, "Send SUB: " << queueName);
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw std::exception();
	}
	this->name = serviceName;
	this->fullName = queueName;
	LOG4CXX_DEBUG(logger, "Sent SUB: " << queueName);
}

EndpointQueue::EndpointQueue(apr_pool_t* pool,
		char* connectionName, int id) {
	shutdown = false;
	lock = new SynchronizableObject();
	
	connection = ConnectionImpl::connect(pool, 10); // TODO allow the timeout to be specified in configuration
	this->pool = pool;

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	char* queueName = (char*) ::malloc(12 + XATMI_SERVICE_NAME_LENGTH + 1 + 5); // /temp-queue/ + serviceName + / + id
	memset(queueName, '\0', 12 + XATMI_SERVICE_NAME_LENGTH + 1 + 5);
	sprintf(queueName, "/temp-queue/%s-%d", connectionName, id);

	stomp_frame frame;
	frame.command = (char*) "SUB";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, queueName);
	apr_hash_set(frame.headers, "receipt", APR_HASH_KEY_STRING, "receipt");
	frame.body_length = -1;
	frame.body = NULL;
	LOG4CXX_DEBUG(logger, "Send SUB: " << queueName);
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw std::exception();
	} else {
	stomp_frame *framed;
	rc = stomp_read(connection, &framed, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not read frame");
		throw new std::exception();
	} else if (strcmp(framed->command, (const char*)"ERROR") == 0) {
		LOG4CXX_DEBUG(logger, (char*) "Got an error: " << framed->body);
		throw new std::exception();
	} else {
		LOG4CXX_DEBUG(logger, (char*) "Subscribed");
	}
	}
	this->name = queueName;
	this->fullName = queueName;
	LOG4CXX_DEBUG(logger, "Sent SUB: " << queueName);
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	LOG4CXX_TRACE(logger, (char*) "destroying" << name);

	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
	delete lock;
	lock = NULL;
	free(fullName);
	LOG4CXX_TRACE(logger, (char*) "destroyed" << name);
}

MESSAGE EndpointQueue::receive(long time) {
	// TODO TIME NOT RESPECTED
	MESSAGE message;
	message.replyto = NULL;
	message.correlationId = -1;
	message.data = NULL;
	message.len = -1;
	message.flags = -1;
	message.control = NULL;
	message.rval = -1;
	message.rcode = -1;

	lock->lock();
	if (!shutdown) {
		stomp_frame *frame;
		LOG4CXX_DEBUG(logger, (char*) "Receivin from: " << name);
		apr_status_t rc = stomp_read(connection, &frame, pool);
		if (rc != APR_SUCCESS) {
			LOG4CXX_DEBUG(logger, "Could not read frame for " << name << ": "
					<< rc << " was the result");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		} else if (strcmp(frame->command, (const char*)"ERROR") == 0) {
			LOG4CXX_ERROR(logger, (char*) "Got an error: " << frame->body);
			setSpecific(TPE_KEY, TSS_TPENOENT);
		} else {
			LOG4CXX_DEBUG(logger, "Received from: " << name  << " Command: " << frame->command << " Body: " << frame->body);
			message.len = frame->body_length;
			message.data = frame->body;
			message.replyto = (const char*) apr_hash_get(frame->headers,
					"reply-to", APR_HASH_KEY_STRING);
			message.control = NULL;
			
			char * correlationId = (char*)apr_hash_get(frame->headers, "messagecorrelationId", APR_HASH_KEY_STRING);
			char * flags = (char*)apr_hash_get(frame->headers, "messageflags", APR_HASH_KEY_STRING);
			char * rval = (char*)apr_hash_get(frame->headers, "messagerval", APR_HASH_KEY_STRING);
			char * rcode = (char*)apr_hash_get(frame->headers, "messagercode", APR_HASH_KEY_STRING);

			message.correlationId = apr_atoi64(correlationId);
			message.flags = apr_atoi64(flags);
			message.rval = apr_atoi64(rval);
			message.rcode = apr_atoi64(rcode);
			//message.control = apr_hash_get(frame->headers, "message.control", APR_HASH_KEY_STRING);
		}
	}
	lock->unlock();
	return message;
}

void EndpointQueue::disconnect() {
	LOG4CXX_DEBUG(logger, (char*) "disconnecting: " << name);
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
	LOG4CXX_DEBUG(logger, (char*) "disconnected: " << name);
}

const char * EndpointQueue::getName() {
	return (const char *) name;
}

const char * EndpointQueue::getFullName() {
	return (const char *)this->fullName;
}
