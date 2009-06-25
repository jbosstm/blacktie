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

#include <string.h>
#include <exception>

#include "apr_strings.h"

#include "malloc.h"
#include "SessionImpl.h"
#include "EndpointQueue.h"
#include "txClient.h"

#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr StompSessionImpl::logger(
		log4cxx::Logger::getLogger("StompSessionImpl"));

StompSessionImpl::StompSessionImpl(char* connectionName, apr_pool_t* pool, int id,
		char* serviceName) {
	LOG4CXX_TRACE(logger, (char*) "constructor ");
	this->id = id;

	connection = NULL;
	connection = StompConnectionImpl::connect(pool, 0); // TODO allow the timeout to be specified in configuration
	this->pool = pool;

	this->canSend = true;
	this->canRecv = true;

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	this->sendTo = (char*) ::malloc(7 + XATMI_SERVICE_NAME_LENGTH + 1);
	memset(this->sendTo, '\0', 7 + XATMI_SERVICE_NAME_LENGTH + 1);
	strcpy(this->sendTo, "/queue/");
	strncat(this->sendTo, serviceName, XATMI_SERVICE_NAME_LENGTH);

	this->toRead = new StompEndpointQueue(this->pool, connectionName, id);
	this->replyTo = toRead->getFullName();
	LOG4CXX_TRACE(logger, "OK service");
}

StompSessionImpl::StompSessionImpl(char* connectionName, apr_pool_t* pool, int id,
		const char* temporaryQueueName) {
	LOG4CXX_TRACE(logger, (char*) "constructor ");
	this->id = id;

	connection = NULL;
	connection = StompConnectionImpl::connect(pool, 0); // TODO allow the timeout to be specified in configuration
	this->pool = pool;

	this->canSend = true;
	this->canRecv = true;

	this->sendTo = ::strdup(temporaryQueueName);

	this->toRead = new StompEndpointQueue(this->pool, connectionName, id);
	this->replyTo = toRead->getFullName();
	LOG4CXX_TRACE(logger, "OK temporary");
}

StompSessionImpl::~StompSessionImpl() {
	::free(this->sendTo);
	delete toRead;

	if (connection) {
		LOG4CXX_TRACE(logger, (char*) "destroying");
		StompConnectionImpl::disconnect(connection, pool);
		LOG4CXX_TRACE(logger, (char*) "destroyed");
		connection = NULL;
	}
}

MESSAGE StompSessionImpl::receive(long time) {
	return toRead->receive(time);
}

bool StompSessionImpl::send(MESSAGE message) {
	bool toReturn = false;
	stomp_frame frame;
	frame.command = (char*) "SEND";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, sendTo);
	apr_hash_set(frame.headers, "receipt", APR_HASH_KEY_STRING, "send");

	frame.body_length = message.len;
	frame.body = message.data;
	if (message.replyto && strcmp(message.replyto, "") != 0) {
		apr_hash_set(frame.headers, "reply-to", APR_HASH_KEY_STRING,
				message.replyto);
	}
	char * correlationId = apr_itoa(pool, message.correlationId);
	char * flags = apr_itoa(pool, message.flags);
	char * rval = apr_itoa(pool, message.rval);
	char * rcode = apr_itoa(pool, message.rcode);
	apr_hash_set(frame.headers, "messagecorrelationId", APR_HASH_KEY_STRING,
			correlationId);
	LOG4CXX_TRACE(logger, "Set the corrlationId: " << correlationId);
	apr_hash_set(frame.headers, "messageflags", APR_HASH_KEY_STRING, flags);
	apr_hash_set(frame.headers, "messagerval", APR_HASH_KEY_STRING, rval);
	apr_hash_set(frame.headers, "messagercode", APR_HASH_KEY_STRING, rcode);
	char* control = serialize_tx((char*) "ots");
	if (control) {
		LOG4CXX_TRACE(logger, "Sending serialized control: " << control);
		apr_hash_set(frame.headers, "messagecontrol", APR_HASH_KEY_STRING, control);
	}

	LOG4CXX_DEBUG(logger, "Send to: " << sendTo << " Command: "
			<< frame.command << " Body: " << frame.body);
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not send frame");
		//setSpecific(TPE_KEY, TSS_TPESYSTEM);
	} else {

		stomp_frame *framed;
		rc = stomp_read(connection, &framed, pool);
		if (rc != APR_SUCCESS) {
			LOG4CXX_ERROR(logger, "Could not send frame");
			//setSpecific(TPE_KEY, TSS_TPESYSTEM);
		} else if (strcmp(framed->command, (const char*) "ERROR") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "Got an error: " << framed->body);
			//setSpecific(TPE_KEY, TSS_TPENOENT);
		} else if (strcmp(framed->command, (const char*) "RECEIPT") == 0) {
			LOG4CXX_DEBUG(logger, (char*) "SEND RECEIPT: "
					<< (char*) apr_hash_get(framed->headers, "receipt-id",
							APR_HASH_KEY_STRING));
			toReturn = true;
		} else {
			LOG4CXX_ERROR(logger, "Didn't get a receipt: " << framed->command
					<< ", " << framed->body);
		}
		LOG4CXX_DEBUG(logger, "Sent to: " << sendTo << " Command: "
				<< frame.command << " Body: " << frame.body);

	}

	LOG4CXX_TRACE(logger, (char*) "freeing data to go: data_togo");
	free(message.data);
	LOG4CXX_TRACE(logger, (char*) "freed");
	return toReturn;
}

void StompSessionImpl::setSendTo(const char* destinationName) {
	this->sendTo = (char*) destinationName;
}

const char* StompSessionImpl::getReplyTo() {
	return replyTo;
}

int StompSessionImpl::getId() {
	return id;
}
