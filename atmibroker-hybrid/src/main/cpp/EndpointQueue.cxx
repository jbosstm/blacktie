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
#include "txClient.h"

log4cxx::LoggerPtr HybridEndpointQueue::logger(log4cxx::Logger::getLogger(
		"HybridEndpointQueue"));

HybridEndpointQueue::HybridEndpointQueue(apr_pool_t* pool, char* serviceName) {
	LOG4CXX_DEBUG(logger, "Creating endpoint queue: " << serviceName);
	this->message = NULL;
	shutdown = false;
	lock = new SynchronizableObject();
	LOG4CXX_DEBUG(logger, "Created lock: " << lock);

	connection = NULL;
	connection = HybridConnectionImpl::connect(pool, 5); // TODO allow the timeout to be specified in configuration
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
	apr_hash_set(frame.headers, "receipt", APR_HASH_KEY_STRING, queueName);
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
		} else if (strcmp(framed->command, (const char*) "ERROR") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "Got an error: " << framed->body);
			throw new std::exception();
		} else if (strcmp(framed->command, (const char*) "RECEIPT") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "svcQ RECEIPT: "
					<< (char*) apr_hash_get(framed->headers, "receipt-id",
							APR_HASH_KEY_STRING));
		} else if (strcmp(framed->command, (const char*) "MESSAGE") == 0) {
			LOG4CXX_DEBUG(
					logger,
					(char*) "Got message before receipt, allow a single receipt later");
			this->message = framed;
			this->receipt = queueName;
		} else {
			LOG4CXX_ERROR(logger, "Didn't get a receipt: " << framed->command
					<< ", " << framed->body);
			throw new std::exception();
		}
	}
	this->name = serviceName;
	this->fullName = queueName;
	this->transactional = true;
	LOG4CXX_DEBUG(logger, "Sent SUB: " << queueName);
}

HybridEndpointQueue::HybridEndpointQueue(apr_pool_t* pool, char* connectionName,
		int id) {
	this->message = NULL;
	shutdown = false;
	lock = new SynchronizableObject();
	LOG4CXX_DEBUG(logger, "Created lock: " << lock);

	connection = NULL;
	connection = HybridConnectionImpl::connect(pool, 10); // TODO allow the timeout to be specified in configuration
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
	apr_hash_set(frame.headers, "receipt", APR_HASH_KEY_STRING, queueName);
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
		} else if (strcmp(framed->command, (const char*) "ERROR") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "Got an error: " << framed->body);
			throw new std::exception();
		} else if (strcmp(framed->command, (const char*) "RECEIPT") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "tmpQ RECEIPT: "
					<< (char*) apr_hash_get(framed->headers, "receipt-id",
							APR_HASH_KEY_STRING));
		} else if (strcmp(framed->command, (const char*) "MESSAGE") == 0) {
			LOG4CXX_DEBUG(
					logger,
					(char*) "Got message before receipt, allow a single receipt later");
			this->message = framed;
			this->receipt = queueName;
		} else {
			LOG4CXX_ERROR(logger, "Didn't get a receipt: " << framed->command
					<< ", " << framed->body);
			throw new std::exception();
		}
	}
	this->name = queueName;
	this->fullName = queueName;
	this->transactional = false;
	LOG4CXX_DEBUG(logger, "Sent SUB: " << queueName);
}

// ~EndpointQueue destructor.
//
HybridEndpointQueue::~HybridEndpointQueue() {
	LOG4CXX_TRACE(logger, (char*) "destroying" << name);

	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
	// Reacquire the lock so we know its safe to delete it
	lock->lock();
	lock->unlock();
	LOG4CXX_TRACE(logger, (char*) "deleting lock");
	delete lock;
	lock = NULL;
	LOG4CXX_TRACE(logger, (char*) "freeing name" << name);
	free(fullName);
	LOG4CXX_TRACE(logger, (char*) "freed name");

	if (connection) {
		LOG4CXX_TRACE(logger, (char*) "destroying");
		HybridConnectionImpl::disconnect(connection, pool);
		LOG4CXX_TRACE(logger, (char*) "destroyed");
		connection = NULL;
	}
}

MESSAGE HybridEndpointQueue::receive(long time) {
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
		stomp_frame *frame = NULL;

		if (this->message) {
			LOG4CXX_DEBUG(logger, "Handing off old message");
			frame = this->message;
			this->message = NULL;
		} else {
			LOG4CXX_DEBUG(logger, (char*) "Receivin from: " << name);
			apr_status_t rc = stomp_read(connection, &frame, pool);
			if (rc != APR_SUCCESS) {
				LOG4CXX_DEBUG(logger, "Could not read frame for " << name
						<< ": " << rc << " was the result");
				setSpecific(TPE_KEY, TSS_TPESYSTEM);
				frame = NULL;
			} else if (strcmp(frame->command, (const char*) "ERROR") == 0) {
				LOG4CXX_ERROR(logger, (char*) "Got an error: " << frame->body);
				setSpecific(TPE_KEY, TSS_TPENOENT);
				frame = NULL;
			} else if (strcmp(frame->command, (const char*) "RECEIPT") == 0) {
				char * receipt = (char*) apr_hash_get(frame->headers,
						"receipt-id", APR_HASH_KEY_STRING);
				if (this->receipt == NULL || strcmp(this->receipt, receipt)
						!= 0) {
					LOG4CXX_ERROR(logger,
							(char*) "read an unexpected receipt for: " << name
									<< ": " << receipt);
					setSpecific(TPE_KEY, TSS_TPESYSTEM);
					frame = NULL;
				} else {
					LOG4CXX_DEBUG(logger, "Handling old receipt");
					this->receipt = NULL;
					rc = stomp_read(connection, &frame, pool);
					if (rc != APR_SUCCESS) {
						LOG4CXX_DEBUG(logger, "Could not read frame for "
								<< name << ": " << rc << " was the result");
						setSpecific(TPE_KEY, TSS_TPESYSTEM);
						frame = NULL;
					} else if (strcmp(frame->command, (const char*) "ERROR")
							== 0) {
						LOG4CXX_ERROR(logger, (char*) "Got an error: "
								<< frame->body);
						setSpecific(TPE_KEY, TSS_TPENOENT);
						frame = NULL;
					} else if (strcmp(frame->command, (const char*) "RECEIPT")
							== 0) {
						LOG4CXX_ERROR(logger, (char*) "read a RECEIPT for: "
								<< name << ": " << (char*) apr_hash_get(
								frame->headers, "receipt-id",
								APR_HASH_KEY_STRING));
						setSpecific(TPE_KEY, TSS_TPESYSTEM);
						frame = NULL;
					} else {
						LOG4CXX_DEBUG(logger, "Message received 2nd attempt");
					}
				}
			} else {
				LOG4CXX_DEBUG(logger, "Message received 1st attempt");
			}
		}
		if (frame != NULL) {
			LOG4CXX_DEBUG(logger, "Received from: " << name << " Command: "
					<< frame->command);
			char* control = (char*) apr_hash_get(frame->headers,
					"messagecontrol", APR_HASH_KEY_STRING);
			LOG4CXX_TRACE(logger, "Extracted control");
			bool unableToAssociateTx = false;
			if (transactional && control && control != NULL && strcmp(control, (const char*) "null") != 0) {
				LOG4CXX_TRACE(logger, "Read a non-null control: " << control << "/");
				if (associate_serialized_tx((char*) "serverAdministration",
						(char*) control) != XA_OK) {
					LOG4CXX_ERROR(logger, "Unable to handle control");
					setSpecific(TPE_KEY, TSS_TPESYSTEM);
					unableToAssociateTx = true;
				}
			}
			if (!unableToAssociateTx) {
				LOG4CXX_TRACE(logger, "Ready to handle message");
				char * correlationId = (char*) apr_hash_get(frame->headers,
						"messagecorrelationId", APR_HASH_KEY_STRING);
				LOG4CXX_TRACE(logger, "Read a correlation ID" << correlationId);
				LOG4CXX_TRACE(logger, "Extracted correlationID");
				char * flags = (char*) apr_hash_get(frame->headers,
						"messageflags", APR_HASH_KEY_STRING);
				LOG4CXX_TRACE(logger, "Extracted flags");
				char * rval = (char*) apr_hash_get(frame->headers,
						"messagerval", APR_HASH_KEY_STRING);
				LOG4CXX_TRACE(logger, "Extracted rval");
				char * rcode = (char*) apr_hash_get(frame->headers,
						"messagercode", APR_HASH_KEY_STRING);
				LOG4CXX_TRACE(logger, "Extracted rcode");

				message.len = frame->body_length;
				LOG4CXX_TRACE(logger, "Set length: " << message.len);
				message.data = frame->body;
				LOG4CXX_TRACE(logger, "Set body");
				message.replyto = (const char*) apr_hash_get(frame->headers,
						"reply-to", APR_HASH_KEY_STRING);
				LOG4CXX_TRACE(logger, "Set replyto: " << message.replyto);
				message.correlationId = apr_atoi64(correlationId);
				LOG4CXX_TRACE(logger, "Set correlationId: " << message.correlationId);
				message.flags = apr_atoi64(flags);
				LOG4CXX_TRACE(logger, "Set flags: " << message.flags);
				message.rval = apr_atoi64(rval);
				LOG4CXX_TRACE(logger, "Set rval: " << message.rval);
				message.rcode = apr_atoi64(rcode);
				LOG4CXX_TRACE(logger, "Set rcode: " << message.rcode);
				message.control = getSpecific(TSS_KEY);
				LOG4CXX_TRACE(logger, "Set control: " << message.control);
			}
		}
	}
	lock->unlock();
	return message;
}

void HybridEndpointQueue::disconnect() {
	LOG4CXX_DEBUG(logger, (char*) "disconnecting: " << name);
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		LOG4CXX_DEBUG(logger, (char*) "Shutdown set: " << name);
		lock->notify();
	}
	lock->unlock();
	LOG4CXX_DEBUG(logger, (char*) "disconnected: " << name);

	if (connection) {
		LOG4CXX_TRACE(logger, (char*) "destroying");
		HybridConnectionImpl::disconnect(connection, pool);
		LOG4CXX_TRACE(logger, (char*) "destroyed");
		connection = NULL;
	}
}

const char * HybridEndpointQueue::getName() {
	return (const char *) name;
}

const char * HybridEndpointQueue::getFullName() {
	return (const char *) this->fullName;
}
