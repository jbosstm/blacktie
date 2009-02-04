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

#include <iostream>
#include <stdio.h>

#include "xatmi.h"
#include "AtmiBrokerClient.h"
#include "EndpointQueue.h"
#include "AtmiBrokerClientXml.h"
#include "userlog.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerClient(Logger::getLogger("AtmiBrokerClient"));

AtmiBrokerClient::AtmiBrokerClient() {
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "constructor ");

	AtmiBrokerClientXml aAtmiBrokerClientXml;
	aAtmiBrokerClientXml.parseXmlDescriptor(&clientServerVector, "CLIENT.xml");

	clientCallbackImpl = new EndpointQueue(client_poa);
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "tmp_servant %p", (void*) clientCallbackImpl);

	client_poa->activate_object(clientCallbackImpl);
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "activated tmp_servant %p", (void*) clientCallbackImpl);

	CORBA::Object_ptr tmp_ref = client_poa->servant_to_reference(clientCallbackImpl);
	clientCallback = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	clientCallbackImpl->setReplyTo(client_orb->object_to_string(clientCallback));
}

AtmiBrokerClient::~AtmiBrokerClient() {
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "destructor ");

	for (std::vector<ClientServerInfo*>::iterator itServer = clientServerVector.begin(); itServer != clientServerVector.end(); itServer++) {
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "next serverName is: %s", (char*) (*itServer)->serverName);
	}
	clientServerVector.clear();
}

EndpointQueue * AtmiBrokerClient::getLocalCallback(int id) {
	return clientCallbackImpl;
}

EndpointQueue * AtmiBrokerClient::getRemoteCallback(int id) {
	return NULL;
}

AtmiBroker::ServiceQueue_ptr AtmiBrokerClient::get_service_queue(const char * serviceName) {
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "get_service_queue: %s", serviceName);

	CosNaming::Name * name = client_default_context->to_name(serviceName);
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "Service name %p", (void*) name);

	AtmiBroker::ServiceQueue_ptr ptr = NULL;

	try {
		CORBA::Object_var tmp_ref = client_name_context->resolve(*name);
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "Service ref %p", (void*) tmp_ref);
		ptr = AtmiBroker::ServiceQueue::_narrow(tmp_ref);
	} catch (...) {
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "Could not access the service queue %p", (void*) name);
	}

	if (CORBA::is_nil(ptr)) {
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "Could not retrieve queue for %s", serviceName);
	} else
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "retrieved  %s queue %p", serviceName, (void*) ptr);

	return AtmiBroker::ServiceQueue::_duplicate(ptr);
}
