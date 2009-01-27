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
#include "AtmiBroker_ClientCallbackImpl.h"

#include "AtmiBrokerClient.h"
#include "userlog.h"

#include "xatmi.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ClientCallbackImpl(Logger::getLogger("AtmiBroker_ClientCallbackImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::ClientCallback*
AtmiBroker_ClientCallbackImpl::_create(PortableServer::POA_ptr the_poa) {
	return new AtmiBroker_ClientCallbackImpl(the_poa);
}

// AtmiBroker_ClientCallbackImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ClientCallbackImpl::AtmiBroker_ClientCallbackImpl(PortableServer::POA_ptr the_poa) :
	synchronizableObject(SynchronizableObject::create(true)) {
}

// ~AtmiBroker_ClientCallbackImpl destructor.
//
AtmiBroker_ClientCallbackImpl::~AtmiBroker_ClientCallbackImpl() {
	// Intentionally empty.
	//
}

// client_callback() -- Implements IDL operation "AtmiBroker::ClientCallback::send_data".
//
void AtmiBroker_ClientCallbackImpl::enqueue_data(CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CORBA::Long revent, const char * id) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback(): called.");

	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    idata = %s", idata.get_buffer());
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    flags = %d", flags);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    id = %s", id);

	MESSAGE message;
	message.rval = rval;
	message.rcode = rcode;
	message.octetSeq = new AtmiBroker::octetSeq(idata);
	message.len = ilen;
	message.flags = flags;
	message.event = revent;
	message.id = id;

	synchronizableObject->lock();
	returnData.push(message);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "notifying");
	synchronizableObject->notify();
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "notified");
	synchronizableObject->unlock();
}

CORBA::Short AtmiBroker_ClientCallbackImpl::dequeue_data(CORBA::Short_out rval, CORBA::Long_out rcode, AtmiBroker::octetSeq_out odata, CORBA::Long_out olen, CORBA::Long_out flags, CORBA::Long_out event) {
	// TODO THIS SHOULD USE THE ID TO CHECK DIFFERENT QUEUES
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "service_response()");

	// Default wait time is 10 seconds
	long time = 10; // TODO Make configurable
	synchronizableObject->lock();
	while (returnData.size() == 0) {
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "waiting for %d", time);
		synchronizableObject->wait(time);
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "out of wait");
		if (!(TPNOTIME & flags)) {
			break;
		}
	}
	synchronizableObject->unlock();

	if (returnData.size() != 0) {
		MESSAGE message = returnData.front();
		returnData.pop();

		rval = message.rval;
		rcode = message.rcode;
		AtmiBroker::octetSeq * aOctetSeq = message.octetSeq;
		odata = new AtmiBroker::octetSeq(*aOctetSeq);
		olen = message.len;
		event = message.event;
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "    fronted octet array %s", (char*) aOctetSeq->get_buffer());
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "   odata = %s", odata->get_buffer());
		return 0;
	} else {
		return -1;
	}
}
