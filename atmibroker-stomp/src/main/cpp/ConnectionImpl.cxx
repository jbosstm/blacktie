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

#include "ConnectionImpl.h"
#include "SessionImpl.h"
#include "AtmiBrokerEnv.h"

log4cxx::LoggerPtr ConnectionImpl::logger(log4cxx::Logger::getLogger(
		"ConnectionImpl"));

ConnectionImpl::ConnectionImpl(char* connectionName) {
	this->connectionName = connectionName;
	apr_status_t rc = apr_initialize();
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not initialize: " << rc);
		throw new std::exception();
	}

	rc = apr_pool_create(&pool, NULL);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not allocate pool: " << rc);
		throw new std::exception();
	}
}

ConnectionImpl::~ConnectionImpl() {
	apr_pool_destroy(pool);
	apr_terminate();
	LOG4CXX_TRACE(logger, "Destroyed");
}

stomp_connection* ConnectionImpl::connect(apr_pool_t* pool, int timeout) {
	stomp_connection* connection = NULL;
	std::string host = AtmiBrokerEnv::get_instance()->getenv(
			(char*) "StompConnectHost");
	std::string port = AtmiBrokerEnv::get_instance()->getenv(
			(char*) "StompConnectPort");
	LOG4CXX_DEBUG(logger, "Connecting to: " << host << ":" << port);
	int portNum = atoi(port.c_str());
	apr_status_t rc = stomp_connect(&connection, host.c_str(), portNum, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not connect: " << host << ", "
				<< port << ": " << rc);
		throw new std::exception();
	}

	apr_socket_opt_set(connection->socket, APR_SO_NONBLOCK, 0);
	apr_socket_timeout_set(connection->socket, 1000000 * timeout);
	LOG4CXX_DEBUG(logger, (char*) "Set socket options");

	std::string usr = AtmiBrokerEnv::get_instance()->getenv(
			(char*) "StompConnectUsr");
	std::string pwd = AtmiBrokerEnv::get_instance()->getenv(
			(char*) "StompConnectPwd");
	LOG4CXX_DEBUG(logger, "Sending CONNECT");
	stomp_frame frame;
	frame.command = (char*) "CONNECT";
	frame.headers = apr_hash_make(pool);
	apr_hash_set(frame.headers, "login", APR_HASH_KEY_STRING, usr.c_str());
	apr_hash_set(frame.headers, "passcode", APR_HASH_KEY_STRING, pwd.c_str());
	frame.body = NULL;
	frame.body_length = -1;
	LOG4CXX_DEBUG(logger, "Connecting...");
	rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not send frame");
		throw new std::exception();
	}

	LOG4CXX_DEBUG(logger, "Reading Response.");
	stomp_frame * frameRead;
	rc = stomp_read(connection, &frameRead, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, (char*) "Could not read frame: " << rc
				<< " from connection");
		throw new std::exception();
	} else {
		LOG4CXX_DEBUG(logger, "Response: " << frameRead->command << ", "
				<< frameRead->body);
		LOG4CXX_DEBUG(logger, "Connected");
	}
	return connection;
}

void ConnectionImpl::disconnect(stomp_connection* connection, apr_pool_t* pool) {
	LOG4CXX_DEBUG(logger, (char*) "Sending DISCONNECT");
	stomp_frame frame;
	frame.command = (char*) "DISCONNECT";
	frame.headers = NULL;
	frame.body_length = -1;
	frame.body = NULL;
	apr_status_t rc = stomp_write(connection, &frame, pool);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not send frame");
	}

	LOG4CXX_DEBUG(logger, "Disconnecting...");
	rc = stomp_disconnect(&connection);
	if (rc != APR_SUCCESS) {
		LOG4CXX_ERROR(logger, "Could not disconnect");
	} else {
		LOG4CXX_DEBUG(logger, "Disconnected");
	}
}

Session* ConnectionImpl::createSession(int id, char * serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	sessionMap[id] = new SessionImpl(connectionName, pool, id, serviceName);
	return sessionMap[id];
}

Session* ConnectionImpl::createSession(int id, const char* temporaryQueueName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	return new SessionImpl(connectionName, pool, id, temporaryQueueName);
}

Destination* ConnectionImpl::createDestination(char* serviceName) {
	return new EndpointQueue(this->pool, serviceName);
}

void ConnectionImpl::destroyDestination(Destination* destination) {
	delete destination;
}

Session* ConnectionImpl::getSession(int id) {
	return sessionMap[id];
}

void ConnectionImpl::closeSession(int id) {
	if (sessionMap[id]) {
		delete sessionMap[id];
		sessionMap[id] = NULL;
	}
}
