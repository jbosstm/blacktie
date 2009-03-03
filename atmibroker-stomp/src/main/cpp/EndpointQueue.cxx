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
#include <exception>

#include "malloc.h"
#include "stomp.h"
#include "EndpointQueue.h"
#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr EndpointQueue::logger(log4cxx::Logger::getLogger("EndpointQueue"));

EndpointQueue::EndpointQueue(stomp_connection* connection, apr_pool_t* pool, char* serviceName) {
	this->connection = connection;
	this->pool = pool;

	char* queueName = (char*) ::malloc(8 + 16);
	strcat(queueName, "/queue/");
	strncat(queueName, serviceName, 15);

	stomp_frame frame;
	frame.command = (char*) "SUB";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, queueName);
	frame.body_length = -1;
	frame.body = NULL;
	LOG4CXX_DEBUG(logger, "Sending SUB");
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw std::exception();
	}
	setName((const char*) queueName);
	LOG4CXX_DEBUG(logger, "OK");
}

EndpointQueue::EndpointQueue(stomp_connection* connection, apr_pool_t* pool, int id) {
	this->connection = connection;
	this->pool = pool;

	char* queueName = (char*) ::malloc(13 + 5);
	sprintf(queueName, "/temp-queue/%d", id);

	stomp_frame frame;
	frame.command = (char*) "SUB";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, queueName);
	frame.body_length = -1;
	frame.body = NULL;
	LOG4CXX_DEBUG(logger, "Sending SUB");
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw std::exception();
	}
	setName((const char*) queueName);
	LOG4CXX_DEBUG(logger, "OK");
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	LOG4CXX_DEBUG(logger, (char*) "destroyed");
}

MESSAGE EndpointQueue::receive(long time) {
	// TODO TIME NOT RESPECTED
	stomp_frame *frame;
	LOG4CXX_DEBUG(logger, (char*) "Reading from: " << name);
	apr_status_t rc = stomp_read(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not read frame");
		throw std::exception();
	}
	LOG4CXX_INFO(logger, "Read: " << frame->command << ", " << frame->body);
	MESSAGE message;
	message.len = frame->body_length;
	message.data = frame->body;
	message.replyto = (const char*) apr_hash_get(frame->headers, "message.replyto", APR_HASH_KEY_STRING);
	//	message.correlationId = (int) apr_hash_get(frame->headers, "message.correlationId", APR_HASH_KEY_STRING);
	//	message.flags = (long) apr_hash_get(frame->headers, "message.flags", APR_HASH_KEY_STRING);
	//	message.control = apr_hash_get(frame->headers, "message.control", APR_HASH_KEY_STRING);
	//	message.rval = (int) apr_hash_get(frame->headers, "message.rval", APR_HASH_KEY_STRING);
	//	message.rcode = (long) apr_hash_get(frame->headers, "message.rcode", APR_HASH_KEY_STRING);
	//	message.event = (long) apr_hash_get(frame->headers, "message.event", APR_HASH_KEY_STRING);
	return message;
}

void EndpointQueue::disconnect() {
	LOG4CXX_DEBUG(logger, (char*) "NOOP");
}

void EndpointQueue::setName(const char* name) {
	this->name = name;
}

const char * EndpointQueue::getName() {
	return name;
}
