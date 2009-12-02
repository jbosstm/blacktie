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

#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"
#include "log4cxx/logger.h"
#include "log4cxx/logmanager.h"

#include "AtmiBrokerServer.h"
#include "ConnectionManager.h"
#include "AtmiBrokerEnv.h"
#include "SymbolLoader.h"
#include "xatmi.h"

log4cxx::LoggerPtr loggerConnectionManager(log4cxx::Logger::getLogger(
		"ConnectionManager"));
extern char server[30];
extern int serverid;

#ifdef IDE_DEBUG
#include "ConnectionImpl.h"
#endif
ConnectionManager::ConnectionManager() {
	LOG4CXX_TRACE(loggerConnectionManager, (char*) "constructor");
	lock = new SynchronizableObject();
}

ConnectionManager::~ConnectionManager() {
	LOG4CXX_TRACE(loggerConnectionManager, (char*) "destructor");

	this->closeConnections();
	delete lock;
}

void ConnectionManager::closeConnections() {
	LOG4CXX_TRACE(loggerConnectionManager, (char*) "closeConnections");

	lock->lock();
	ConnectionMap::iterator it;
	for (it = manager.begin(); it != manager.end(); it++) {
		delete (*it).second;
	}
	manager.clear();
	lock->unlock();
}

Connection*
ConnectionManager::getConnection(char* serviceName, char* side) {
	char* transportLibrary;
	char adm[XATMI_SERVICE_NAME_LENGTH + 1];
	ACE_OS::snprintf(adm, XATMI_SERVICE_NAME_LENGTH + 1, "%s_ADMIN_%d", server,
			serverid);

	if (strcmp(serviceName, adm) == 0) {
#ifdef WIN32
		transportLibrary = (char*) "atmibroker-hybrid.dll";
#else
		transportLibrary = (char*) "libatmibroker-hybrid.so";
#endif
	} else {
		AtmiBrokerEnv* env = AtmiBrokerEnv::get_instance();
		transportLibrary = env->getTransportLibrary(serviceName);
		AtmiBrokerEnv::discard_instance();
		if (transportLibrary == NULL) {
			LOG4CXX_WARN(loggerConnectionManager, (char*) "service "
					<< serviceName
					<< " is not configured in the Environment.xml");
			throw std::exception();
		}
	}

	LOG4CXX_DEBUG(loggerConnectionManager, (char*) "service " << serviceName
			<< " transport is " << transportLibrary);
	std::string key = side;
	key.append("/");
	key.append(transportLibrary);

	lock->lock();
	ConnectionMap::iterator it;
	it = manager.find(key);

	if (it != manager.end()) {
		LOG4CXX_DEBUG(loggerConnectionManager, (char*) "find " << serviceName
				<< " Connection in map " << (*it).second);
		lock->unlock();
		return (*it).second;
	} else {
#ifdef IDE_DEBUG
		Connection* connection = new HybridConnectionImpl(side);
		manager.insert(ConnectionMap::value_type(key, connection));
		LOG4CXX_DEBUG(loggerConnectionManager, (char*) "insert service " << key << " connection " << connection);
		lock->unlock();
		return connection;
#else
		connection_factory_t* connectionFactory =
				(connection_factory_t*) ::lookup_symbol(transportLibrary,
						"connectionFactory");
		if (connectionFactory != NULL) {
			Connection* connection = connectionFactory->create_connection(side);
			manager.insert(ConnectionMap::value_type(key, connection));
			LOG4CXX_DEBUG(loggerConnectionManager, (char*) "insert service "
					<< key << " connection " << connection);
			lock->unlock();
			return connection;
		}
#endif
	}

	LOG4CXX_WARN(loggerConnectionManager,
			(char*) "can not create connection for service " << serviceName);
	lock->unlock();
	return NULL;
}

Connection*
ConnectionManager::getClientConnection(char* serviceName) {
	return getConnection(serviceName, (char*) "client");
}

Connection*
ConnectionManager::getServerConnection(char* serviceName) {
	return getConnection(serviceName, (char*) "server");
}
