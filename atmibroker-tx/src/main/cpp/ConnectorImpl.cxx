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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#include "ConnectorImpl.h"
#include "LocalResourceManagerCache.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerConnector(Logger::getLogger("Connector"));

ConnectorImpl::ConnectorImpl() {
	LOG4CXX_LOGLS(loggerConnector, Level::getDebug(), (char*) "constructor");
}

ConnectorImpl::~ConnectorImpl() {
	LOG4CXX_LOGLS(loggerConnector, Level::getDebug(), (char*) "destructor");
}

XA::ResourceManager_ptr
#ifdef TAO_COMP
ConnectorImpl::create_resource_manager (const char * resource_manager_name,
		//											XA::XASwitch xa_switch,
		XA::XASwitch_ptr xa_switch,
		const char * open_string,
		const char * close_string,
		XA::ThreadModel thread_model,
		CORBA::Boolean automatic_association,
		CORBA::Boolean dynamic_registration_optimization,
		XA::CurrentConnection_out current_connection)
#else
//#elif ORBIX_COMP
ConnectorImpl::create_resource_manager(const char * resource_manager_name, const XA::XASwitch& xa_switch, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, XA::CurrentConnection_out current_connection)
#endif
		throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) "create_resource_manager ENTERED");

	LocalResourceManager * aLocalResourceManager = LocalResourceManagerCache::getLocalResourceManagerCache()->find_local_resource_manager(resource_manager_name, open_string, close_string, thread_model, automatic_association, dynamic_registration_optimization);

	ConnectorDataStruct* aConnectorDataStruct = (ConnectorDataStruct*) malloc(sizeof(ConnectorDataStruct));

	aConnectorDataStruct->currentConnectionImpl = new CurrentConnectionImpl(*aLocalResourceManager);
	current_connection = aConnectorDataStruct->currentConnectionImpl;

	aConnectorDataStruct->localResourceManager = aLocalResourceManager;

	connectorDataQueue.push_back(aConnectorDataStruct);

	LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) "create_resource_manager FINISHED ");

	return aLocalResourceManager;
}

XA::CurrentConnection_ptr
#ifdef TAO_COMP
ConnectorImpl::connect_to_resource_manager (XA::ResourceManager_ptr rm,
		//							XA::XASwitch xa_switch,
		XA::XASwitch_ptr xa_switch,
		const char * open_string,
		const char * close_string,
		XA::ThreadModel thread_model,
		CORBA::Boolean automatic_association,
		CORBA::Boolean dynamic_registration_optimization)
#else
//#elif ORBIX_COMP
ConnectorImpl::connect_to_resource_manager(XA::ResourceManager_ptr rm, const XA::XASwitch& xa_switch, const char * open_string, const char * close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization)
#endif
		throw(CORBA::SystemException) {
	LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) "connect_to_resource_manager ENTERED");

	for (std::deque<ConnectorDataStruct*>::iterator itConnectorDataStruct = connectorDataQueue.begin(); itConnectorDataStruct != connectorDataQueue.end(); itConnectorDataStruct++) {
		LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) "next " << (*itConnectorDataStruct)->localResourceManager);
		if (rm->_is_equivalent((*itConnectorDataStruct)->localResourceManager)) {
			LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) " found matching resource manager");
			return (*itConnectorDataStruct)->currentConnectionImpl;
		}
	}
	LOG4CXX_LOGLS(loggerConnector, Level::getInfo(), (char*) "connect_to_resource_manager FINISHED ");
	return XA::CurrentConnection::_nil();
}

