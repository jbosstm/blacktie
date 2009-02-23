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
#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include <string.h>
#include "ConnectionImpl.h"
#include "SessionImpl.h"
#include "AtmiBrokerOTS.h"
#include "OrbManagement.h"

log4cxx::LoggerPtr ConnectionImpl::logger(log4cxx::Logger::getLogger("ConnectionImpl"));

ConnectionImpl::ConnectionImpl(char* connectionName) {
	LOG4CXX_DEBUG(logger, (char*) "constructor");
	this->connection = AtmiBrokerOTS::init_orb(connectionName);
}

ConnectionImpl::~ConnectionImpl() {
	LOG4CXX_DEBUG(logger, (char*) "destructor");
	shutdownBindings(this->connection);
}

Session* ConnectionImpl::createSession(int id, char * serviceName) {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	return new SessionImpl(this, id, serviceName);
}

Session* ConnectionImpl::createSession() {
	LOG4CXX_DEBUG(logger, (char*) "createSession");
	return new SessionImpl(this, -1);
}

int ConnectionImpl::block() {
	int toReturn = 0;
	LOG4CXX_INFO(logger, "Server waiting for requests...");
	try {
		((CORBA::ORB_ptr) this->connection->orbRef)->run();
	} catch (CORBA::Exception& e) {
		LOG4CXX_ERROR(logger, "Unexpected CORBA exception: %s" << e._name());
		toReturn = -1;
	} catch (...) {
		LOG4CXX_ERROR(logger, "Unexpected exception");
		toReturn = -1;
	}
	return toReturn;
}

CORBA_CONNECTION* ConnectionImpl::getRealConnection() {
	LOG4CXX_DEBUG(logger, (char*) "getRealConnection");
	return this->connection;
}
