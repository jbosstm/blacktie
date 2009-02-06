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
#include <tao/ORB.h>
#include "AtmiBrokerC.h"
#include "ReceiverImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr ReceiverImpl::logger(log4cxx::Logger::getLogger("ReceiverImpl"));

ReceiverImpl::ReceiverImpl(CONNECTION* connection) {
	PortableServer::POA_ptr poa = (PortableServer::POA_ptr) connection->callback_poa;
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	EndpointQueue* endpointQueue = new EndpointQueue(poa);
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "tmp_servant " << endpointQueue);
	poa->activate_object(endpointQueue);
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "activated tmp_servant " << endpointQueue);
	CORBA::Object_ptr tmp_ref = poa->servant_to_reference(endpointQueue);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	endpointQueue->setName(orb->object_to_string(queue));
	this->destination = endpointQueue;
}

ReceiverImpl::~ReceiverImpl() {
}

MESSAGE ReceiverImpl::receive(long flags) {
	return destination->receive(flags);
}

Destination* ReceiverImpl::getDestination() {
	return destination;
}
