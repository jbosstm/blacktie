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
#include <orbsvcs/CosNamingS.h>
#include "EndpointQueue.h"
#include "AtmiBrokerClient.h"
#include "userlog.h"
#include "xatmi.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerEndpointQueue(log4cxx::Logger::getLogger("EndpointQueue"));

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

EndpointQueue::EndpointQueue(void* connection_orb, char * callback_ior) {
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection_orb;
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "EndpointQueue: %s", callback_ior);
	CORBA::Object_var tmp_ref = orb->string_to_object(callback_ior);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "connected to %s", callback_ior);
}

EndpointQueue::EndpointQueue(void* connection_context, void* connection_name_context, const char * serviceName) {
	CosNaming::NamingContextExt_ptr context = (CosNaming::NamingContextExt_ptr) connection_context;
	CosNaming::NamingContext_ptr name_context = (CosNaming::NamingContext_ptr) connection_name_context;
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "EndpointQueue: %s", serviceName);
	CosNaming::Name * name = context->to_name(serviceName);
	CORBA::Object_var tmp_ref = name_context->resolve(*name);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "connected to %s", serviceName);
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	// Intentionally empty.
	//
}

void EndpointQueue::send(MESSAGE message) {
	unsigned char * data_togo = (unsigned char *) malloc(message.len);
	memcpy(data_togo, message.data, message.len);
	AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(message.len, message.len, data_togo, true);
	remoteEndpoint->send(message.replyto, message.rval, message.rcode, aOctetSeq, message.len, message.correlationId, message.flags);
	aOctetSeq = NULL;
}

void EndpointQueue::send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "client_callback(): called.");
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    idata = %s", idata.get_buffer());
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    ilen = %d", ilen);
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "client_callback():    flags = %d", flags);

	MESSAGE message;
	message.correlationId = correlationId;
	message.data = (char*) malloc(sizeof(char*) * ilen);
	memcpy(message.data, (char*) idata.get_buffer(), ilen);
	message.flags = flags;
	message.len = ilen;
	message.rcode = rcode;
	message.replyto = replyto_ior;
	message.rval = rval;

	lock->lock();
	returnData.push(message);
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "notifying");
	lock->notify();
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "notified");
	lock->unlock();
}

MESSAGE EndpointQueue::receive(long flags) {
	// TODO THIS SHOULD USE THE ID TO CHECK DIFFERENT QUEUES
	userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "service_response()");

	// Default wait time is 10 seconds
	long time = 10; // TODO Make configurable
	MESSAGE message;
	message.data = NULL;
	lock->lock();
	while (returnData.size() == 0) {
		userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "waiting for %d", time);
		lock->wait(time);
		userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "out of wait");
		if (!(TPNOTIME & flags)) {
			break;
		}
	}
	if (returnData.size() != 0) {
		message = returnData.front();
		returnData.pop();
		userlog(log4cxx::Level::getDebug(), loggerEndpointQueue, (char*) "returning %p", message);
	}
	lock->unlock();
	return message;
}

void EndpointQueue::disconnect() throw (CORBA::SystemException ) {
	userlog(log4cxx::Level::getError(), loggerEndpointQueue, (char*) "disconnect unimplemented");
	if (remoteEndpoint) {
		remoteEndpoint->disconnect();
	}
}

void EndpointQueue::setName(const char* name) {
	this->name = name;
}

const char * EndpointQueue::getName() {
	return name;
}
