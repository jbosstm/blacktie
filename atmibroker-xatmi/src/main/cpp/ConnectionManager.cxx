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

#include "ConnectionManager.h"
#include "AtmiBrokerEnv.h"
#include "SymbolLoader.h"

log4cxx::LoggerPtr loggerConnectionManager(log4cxx::Logger::getLogger(
		"ConnectionManager"));

ConnectionManager::ConnectionManager()
{
	LOG4CXX_TRACE(loggerConnectionManager, (char*) "constructor");
}

ConnectionManager::~ConnectionManager()
{
	LOG4CXX_TRACE(loggerConnectionManager, (char*) "destructor");

	ConnectionMap::iterator it;
	for(it = manager.begin(); it != manager.end(); it ++) {
		delete (*it).second;
	}
	manager.clear();
}

Connection*
ConnectionManager::getConnection(char* serviceName, char* side)
{
	char* transportLibrary = AtmiBrokerEnv::get_instance()->getTransportLibrary(serviceName);
	if(transportLibrary == NULL){
		LOG4CXX_WARN(loggerConnectionManager, (char*) "service " << serviceName << " has not transportLibrary config");
		throw std::exception();
	}

	LOG4CXX_DEBUG(loggerConnectionManager, (char*) "service " << serviceName << " transport is " << transportLibrary);
	std::string key = side;
	key.append("/");
	key.append(transportLibrary);

	ConnectionMap::iterator it;
	it = manager.find(key);
	
	if(it != manager.end()) {
		LOG4CXX_DEBUG(loggerConnectionManager, (char*) "find " << serviceName << " Connection in map " << (*it).second);
		return (*it).second;
	} else {
		connection_factory_t* connectionFactory = (connection_factory_t*) ::lookup_symbol(transportLibrary, "connectionFactory");
		if (connectionFactory != NULL) {
			Connection* connection = connectionFactory->create_connection(side);
			manager.insert(ConnectionMap::value_type(key, connection));
			LOG4CXX_DEBUG(loggerConnectionManager, (char*) "insert service " << key << " connection " << connection);
			return connection;
		}
	}

	LOG4CXX_WARN(loggerConnectionManager, (char*) "can not create connection for service " << serviceName);
	return NULL;
}

Connection*
ConnectionManager::getClientConnection(char* serviceName)
{
	return getConnection(serviceName, (char*)"client");
}

Connection*
ConnectionManager::getServerConnection(char* serviceName)
{
	return getConnection(serviceName, (char*)"server");
}
