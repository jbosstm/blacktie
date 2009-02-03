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

//
// Servant which implements the AtmiBroker::ClientCallback interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include <stdlib.h>
#include <iostream>
#include "EndpointQueue.h"

#include "AtmiBrokerClient.h"
#include "userlog.h"

#include "xatmi.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerEndpointQueue(Logger::getLogger("EndpointQueue"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::EndpointQueue*
EndpointQueue::_create(PortableServer::POA_ptr the_poa) {
	return new EndpointQueue(the_poa);
}

// EndpointQueue constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
EndpointQueue::EndpointQueue(PortableServer::POA_ptr the_poa) :
	lock(SynchronizableObject::create(false)) {
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	// Intentionally empty.
	//
}

// client_callback() -- Implements IDL operation "AtmiBroker::ClientCallback::send_data".
//
void EndpointQueue::send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CORBA::Long revent) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "client_callback(): called.");

	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    idata = %s", idata.get_buffer());
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    flags = %d", flags);

	MESSAGE message;
	message.replyto_ior = replyto_ior;
	message.rval = rval;
	message.rcode = rcode;
	message.idata = (char*) malloc(sizeof(char*) * ilen);
	memcpy(message.idata, (char*) idata.get_buffer(), ilen);
	message.ilen = ilen;
	message.flags = flags;
	message.event = revent;

	lock->lock();
	returnData.push(message);
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "notifying");
	lock->notify();
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "notified");
	lock->unlock();
}

MESSAGE EndpointQueue::receive(long flags) {
	// TODO THIS SHOULD USE THE ID TO CHECK DIFFERENT QUEUES
	userlog(Level::getDebug(), loggerEndpointQueue, (char*) "service_response()");

	// Default wait time is 10 seconds
	long time = 10; // TODO Make configurable
	MESSAGE message;
	message.idata = NULL;
	lock->lock();
	while (returnData.size() == 0) {
		userlog(Level::getDebug(), loggerEndpointQueue, (char*) "waiting for %d", time);
		lock->wait(time);
		userlog(Level::getDebug(), loggerEndpointQueue, (char*) "out of wait");
		if (!(TPNOTIME & flags)) {
			break;
		}
	}
	if (returnData.size() != 0) {
		message = returnData.front();
		returnData.pop();
		userlog(Level::getDebug(), loggerEndpointQueue, (char*) "returning %p", message);
	}
	lock->unlock();
	return message;
}

void EndpointQueue::disconnect() throw (CORBA::SystemException ) {
}

void EndpointQueue::setReplyTo(const char* replyTo) {
	m_replyTo = replyTo;
}

const char * EndpointQueue::getReplyTo() {
	return m_replyTo;
}
