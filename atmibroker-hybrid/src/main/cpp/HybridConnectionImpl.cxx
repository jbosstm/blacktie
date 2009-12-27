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
#include <stdlib.h>
#include <string.h>
#include <exception>

#include "HybridConnectionImpl.h"
#include "HybridSessionImpl.h"
#include "AtmiBrokerEnv.h"
#include "HybridStompEndpointQueue.h"

#include "OrbManagement.h"
#include "txx.h"
#include "AtmiBrokerPoaFac.h"

log4cxx::LoggerPtr HybridConnectionImpl::logger(log4cxx::Logger::getLogger(
		"HybridConnectionImpl"));

HybridConnectionImpl::HybridConnectionImpl(char* connectionName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor: " << connectionName);
	this->connectionName = connectionName;
	apr_status_t rc = apr_initialize();
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not initialize: " << rc);
		throw new std::exception();
	}
	LOG4CXX_TRACE(logger, (char*) "Initialized apr");

	rc = apr_pool_create(&pool, NULL);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not allocate pool: " << rc);
		throw new std::exception();
	}
	LOG4CXX_TRACE(logger, (char*) "Pool created");

	this->connection = (CORBA_CONNECTION *) initOrb(connectionName);
}

HybridConnectionImpl::~HybridConnectionImpl() {
	std::map<int, HybridSessionImpl*>::iterator i;

	for (i = sessionMap.begin(); i != sessionMap.end(); ++i) {
		closeSession((*i).first);
	}

	LOG4CXX_DEBUG(logger, (char*) "destructor: " << connectionName);
	shutdownBindings(this->connection);

	apr_pool_destroy( pool);
	//apr_terminate();
	LOG4CXX_TRACE(logger, "Destroyed");
}

bool HybridConnectionImpl::requiresAdminCall() {
	return true;
}

stomp_connection* HybridConnectionImpl::connect(apr_pool_t* pool, int timeout) {
	LOG4CXX_DEBUG(logger, "connect:" << timeout);
	stomp_connection* connection = NULL;
	std::string host = mqConfig.host;
	int portNum = mqConfig.port;
	LOG4CXX_DEBUG(logger, "connect to: " << host << ":" << portNum);
	apr_status_t rc = stomp_connect(&connection, host.c_str(), portNum, pool);
	if (rc != APR_SUCCESS) {
		char errbuf[256];
		apr_strerror(rc, errbuf, sizeof(errbuf));
		LOG4CXX_ERROR(logger, (char*) "Connection failed: " << host << ", "
				<< portNum << ": " << rc << ": " << errbuf);
		free(errbuf);
		disconnect(connection, pool);
	} else {
		if (timeout > 0) {
			apr_socket_opt_set(connection->socket, APR_SO_NONBLOCK, 0);
			apr_socket_timeout_set(connection->socket, 1000000 * timeout);
			LOG4CXX_DEBUG(logger, (char*) "Set socket options");
		}

		std::string usr = mqConfig.user;
		std::string pwd = mqConfig.pwd;
		LOG4CXX_DEBUG(logger, "Sending CONNECT");
		stomp_frame frame;
		frame.command = (char*) "CONNECT";
		frame.headers = apr_hash_make(pool);
		apr_hash_set(frame.headers, "login", APR_HASH_KEY_STRING, usr.c_str());
		apr_hash_set(frame.headers, "passcode", APR_HASH_KEY_STRING,
				pwd.c_str());
		frame.body = NULL;
		frame.body_length = -1;
		LOG4CXX_DEBUG(logger, "Connecting...");
		rc = stomp_write(connection, &frame, pool);
		if (rc != APR_SUCCESS) {
			char errbuf[256];
			apr_strerror(rc, errbuf, sizeof(errbuf));
			LOG4CXX_ERROR(logger, (char*) "Could not send frame: " << rc
					<< ": " << errbuf);
			free(errbuf);
			disconnect(connection, pool);
		} else {
			LOG4CXX_DEBUG(logger, "Reading Response.");
			stomp_frame * frameRead = NULL;
			try {
				rc = stomp_read(connection, &frameRead, pool);
				if (rc != APR_SUCCESS) {
					char errbuf[256];
					apr_strerror(rc, errbuf, sizeof(errbuf));
					LOG4CXX_ERROR(logger, (char*) "Could not read frame: "
							<< rc << " from connection: " << errbuf);
					free(errbuf);
					disconnect(connection, pool);
				} else {
					LOG4CXX_DEBUG(logger, "Response: " << frameRead->command
							<< ", " << frameRead->body);
					LOG4CXX_DEBUG(logger, "Connected");
				}
			} catch (...) {
				LOG4CXX_ERROR(logger, (char*) "Could not read from socket");
			}
		}
	}
	return connection;
}

void HybridConnectionImpl::disconnect(stomp_connection* connection,
		apr_pool_t* pool) {
	if (connection != NULL) {
		LOG4CXX_DEBUG(logger, (char*) "HybridConnectionImpl::disconnect");
		stomp_frame frame;
		frame.command = (char*) "DISCONNECT";
		frame.headers = NULL;
		frame.body_length = -1;
		frame.body = NULL;
		LOG4CXX_TRACE(logger, (char*) "Sending DISCONNECT" << connection
				<< "pool" << pool);
		apr_status_t rc = stomp_write(connection, &frame, pool);
		LOG4CXX_TRACE(logger, (char*) "Sent DISCONNECT");
		if (rc != APR_SUCCESS) {
			char errbuf[256];
			apr_strerror(rc, errbuf, sizeof(errbuf));
			LOG4CXX_ERROR(logger, "Could not send frame: " << rc << ": "
					<< errbuf);
			free(errbuf);
		}

		LOG4CXX_DEBUG(logger, "Disconnecting...");
		rc = stomp_disconnect(&connection);
		if (rc != APR_SUCCESS) {
			char errbuf[256];
			apr_strerror(rc, errbuf, sizeof(errbuf));
			LOG4CXX_ERROR(logger, "Could not disconnect: " << errbuf);
			free(errbuf);
		} else {
			LOG4CXX_DEBUG(logger, "Disconnected");
		}
	}
}

Session* HybridConnectionImpl::createSession(int id, char * serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession serviceName: " << serviceName);
	sessionMap[id] = new HybridSessionImpl(this->connectionName,
			this->connection, pool, id, serviceName);
	return sessionMap[id];
}

Session* HybridConnectionImpl::createSession(int id,
		const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession temporaryQueueName: "
			<< temporaryQueueName);
	sessionMap[id] = new HybridSessionImpl(this->connectionName,
			this->connection, this->pool, id, temporaryQueueName);
	return sessionMap[id];
}

Destination* HybridConnectionImpl::createDestination(char* serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createDestination" << serviceName);
	return new HybridStompEndpointQueue(this->pool, serviceName);
}

void HybridConnectionImpl::destroyDestination(Destination* destination) {
	HybridStompEndpointQueue* queue =
			dynamic_cast<HybridStompEndpointQueue*> (destination);
	delete queue;
}

Session* HybridConnectionImpl::getSession(int id) {
	return sessionMap[id];
}

void HybridConnectionImpl::closeSession(int id) {
	if (sessionMap[id]) {
		HybridSessionImpl* session = sessionMap[id];
		delete session;
		sessionMap[id] = NULL;
	}
}
