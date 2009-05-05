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

#include "malloc.h"
#include "EndpointQueue.h"
#include "ThreadLocalStorage.h"

char* fTPERESET = (char*) "0";
char* fTPEBADDESC = (char*) "2";
char* fTPEBLOCK = (char*) "3";
char* fTPEINVAL = (char*) "4";
char* fTPELIMIT = (char*) "5";
char* fTPENOENT = (char*) "6";
char* fTPEOS = (char*) "7";
char* fTPEPROTO = (char*) "9";
char* fTPESVCERR = (char*) "10";
char* fTPESVCFAIL = (char*) "11";
char* fTPESYSTEM = (char*) "12";
char* fTPETIME = (char*) "13";
char* fTPETRAN = (char*) "14";
char* fTPGOTSIG = (char*) "15";
char* fTPEITYPE = (char*) "17";
char* fTPEOTYPE = (char*) "18";
char* fTPEEVENT = (char*) "22";
char* fTPEMATCH = (char*) "23";


log4cxx::LoggerPtr EndpointQueue::logger(log4cxx::Logger::getLogger(
		"EndpointQueue"));

EndpointQueue::EndpointQueue(stomp_connection* connection, apr_pool_t* pool,
		char* serviceName) {
	shutdown = false;
	lock = new SynchronizableObject();

	this->connection = connection;
	this->pool = pool;

	char* queueName = (char*) ::malloc(8 + 16);
	memset(queueName, '\0', 25);
	strcpy(queueName, "/queue/");
	strncat(queueName, serviceName, 15);

	stomp_frame frame;
	frame.command = (char*) "SUB";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, queueName);
	frame.body_length = -1;
	frame.body = NULL;
	//LOG4CXX_DEBUG(logger, (char*) "Sending SUB");
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw std::exception();
	}
	this->name = serviceName;
	this->fullName = (const char*) queueName;
	LOG4CXX_DEBUG(logger, "OK");
}

EndpointQueue::EndpointQueue(stomp_connection* connection, apr_pool_t* pool,
		char* connectionName, int id) {
	shutdown = false;
	lock = new SynchronizableObject();

	this->connection = connection;
	this->pool = pool;

	char* queueName = (char*) ::malloc(12 + 15 + 1 + 5); // /temp-queue/ + serviceName + / + id
	memset(queueName, '\0', 33);
	sprintf(queueName, "/temp-queue/%s-%d", connectionName, id);

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
	this->name = queueName;
	this->fullName = (const char*) queueName;
	LOG4CXX_DEBUG(logger, "OK");
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	LOG4CXX_DEBUG(logger, (char*) "destroying" << name);

	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
	delete lock;
	lock = NULL;
	LOG4CXX_DEBUG(logger, (char*) "destroyed" << name);
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
	message.event = -1;

	lock->lock();
	if (!shutdown) {
		stomp_frame *frame;
		LOG4CXX_DEBUG(logger, (char*) "Reading from: " << name);
		apr_status_t rc = stomp_read(connection, &frame, pool);
		if (rc != APR_SUCCESS) {
			LOG4CXX_ERROR(logger, "Could not read frame for " << name << ": "
					<< rc << " was the result");
			setSpecific(TPE_KEY, fTPESYSTEM);
		} else if (strcmp(frame->command, (const char*)"ERROR") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "Got an error: " << frame->body);
			setSpecific(TPE_KEY, fTPENOENT);
		} else {
			LOG4CXX_INFO(logger, "Read: " << frame->command << ", "
					<< frame->body);
			message.len = frame->body_length;
			message.data = frame->body;
			message.replyto = (const char*) apr_hash_get(frame->headers,
					"reply-to", APR_HASH_KEY_STRING);
			message.control = NULL;
			//	message.correlationId = (int) apr_hash_get(frame->headers, "message.correlationId", APR_HASH_KEY_STRING);
			//	message.flags = (long) apr_hash_get(frame->headers, "message.flags", APR_HASH_KEY_STRING);
			//	message.control = apr_hash_get(frame->headers, "message.control", APR_HASH_KEY_STRING);
			//	message.rval = (int) apr_hash_get(frame->headers, "message.rval", APR_HASH_KEY_STRING);
			//	message.rcode = (long) apr_hash_get(frame->headers, "message.rcode", APR_HASH_KEY_STRING);
			//	message.event = (long) apr_hash_get(frame->headers, "message.event", APR_HASH_KEY_STRING);
		}
	}
	lock->unlock();
	return message;
}

void EndpointQueue::disconnect() {
	LOG4CXX_DEBUG(logger, (char*) "disconnecting");
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
}

const char * EndpointQueue::getName() {
	return (const char *) name;
}

const char * EndpointQueue::getFullName() {
	return this->fullName;
}
