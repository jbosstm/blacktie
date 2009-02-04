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
#include "AtmiBroker.h"
#include "AtmiBrokerClient.h"
#include "EndpointQueue.h"
#include "Sender.h"
#include "SenderImpl.h"
#include "Receiver.h"
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

	EndpointQueue* endpointQueue = new EndpointQueue(client_poa);
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "tmp_servant %p", (void*) endpointQueue);
	client_poa->activate_object(endpointQueue);
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "activated tmp_servant %p", (void*) endpointQueue);
	CORBA::Object_ptr tmp_ref = client_poa->servant_to_reference(endpointQueue);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	endpointQueue->setReplyTo(client_orb->object_to_string(queue));
	queueReceiver = endpointQueue;
	id = 0;
}

AtmiBrokerClient::~AtmiBrokerClient() {
	userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "destructor ");

	for (std::vector<ClientServerInfo*>::iterator itServer = clientServerVector.begin(); itServer != clientServerVector.end(); itServer++) {
		userlog(Level::getDebug(), loggerAtmiBrokerClient, (char*) "next serverName is: %s", (char*) (*itServer)->serverName);
	}
	clientServerVector.clear();
}

Session* AtmiBrokerClient::createSession() {
	return this;
}

Session* AtmiBrokerClient::getSession(int* id) {
	return this;
}

void AtmiBrokerClient::getId(int& id) {
	id = this->id;
}

void AtmiBrokerClient::setReplyTo(char * replyTo) {
	this->replyTo = replyTo;
}

Receiver * AtmiBrokerClient::getReceiver() {
	return queueReceiver;
}

Sender * AtmiBrokerClient::getSender() {
	return new SenderImpl(client_orb, replyTo);
}
