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
#include <orbsvcs/CosNamingS.h>
#endif

#include "EndpointQueue.h"
#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr EndpointQueue::logger(log4cxx::Logger::getLogger("EndpointQueue"));

// EndpointQueue constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
EndpointQueue::EndpointQueue(CORBA_CONNECTION* connection) {
	shutdown = false;
	lock = new SynchronizableObject();

	PortableServer::POA_ptr poa = (PortableServer::POA_ptr) connection->callback_poa;
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	LOG4CXX_DEBUG(logger, (char*) "tmp_servant " << this);
	poa->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_ptr tmp_ref = poa->servant_to_reference(this);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	setName(orb->object_to_string(queue));
}

EndpointQueue::EndpointQueue(CORBA_CONNECTION* connection, void* poa, char* serviceName) {
	shutdown = false;
	thePoa = poa;
	PortableServer::POA_ptr aFactoryPoaPtr = (PortableServer::POA_ptr) poa;
	lock = new SynchronizableObject();
	aFactoryPoaPtr->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_var tmp_ref = aFactoryPoaPtr->servant_to_reference(this);
	CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serviceName);
	((CosNaming::NamingContext_ptr) connection->name_ctx)->bind(*name, tmp_ref);
	setName("servicequeue");
}

// ~EndpointQueue destructor.
//
EndpointQueue::~EndpointQueue() {
	LOG4CXX_DEBUG(logger, (char*) "destroy called");

	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
	delete lock;
	lock = NULL;
}

void EndpointQueue::send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(logger, (char*) "send called.");
	LOG4CXX_DEBUG(logger, (char*) "send idata = " << replyto_ior);
	LOG4CXX_DEBUG(logger, (char*) "send idata = " << idata.get_buffer());
	LOG4CXX_DEBUG(logger, (char*) "send ilen = " << ilen);
	LOG4CXX_DEBUG(logger, (char*) "send flags = " << flags);

	MESSAGE message;
	message.correlationId = correlationId;
	message.data = (char*) malloc(sizeof(char*) * ilen);
	memcpy(message.data, (char*) idata.get_buffer(), ilen);
	message.flags = flags;
	message.len = ilen;
	message.rcode = rcode;
	message.replyto = strdup(replyto_ior);
	message.rval = rval;
	message.control = getSpecific(TSS_KEY);

	lock->lock();
	returnData.push(message);
	LOG4CXX_DEBUG(logger, (char*) "notifying");
	lock->notify();
	LOG4CXX_DEBUG(logger, (char*) "notified");
	lock->unlock();
}

MESSAGE EndpointQueue::receive(bool noWait) {
	// TODO THIS SHOULD USE THE ID TO CHECK DIFFERENT QUEUES
	LOG4CXX_DEBUG(logger, (char*) "service_response()");

	// Default wait time is 10 seconds
	long time = 10; // TODO Make configurable
	if (!noWait) {
		time = 0;
	}
	MESSAGE message;
	message.data = NULL;
	lock->lock();
	if (!shutdown) {
		if (returnData.size() == 0) {
			LOG4CXX_DEBUG(logger, (char*) "waiting for %d" << time);
			lock->wait(time);
			LOG4CXX_DEBUG(logger, (char*) "out of wait");
		}
		if (returnData.size() > 0) {
			message = returnData.front();
			returnData.pop();
			LOG4CXX_DEBUG(logger, (char*) "returning message");
		}
	}
	lock->unlock();
	return message;
}

void EndpointQueue::disconnect() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(logger, (char*) "disconnect");
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notify();
	}
	lock->unlock();
}

void EndpointQueue::setName(const char* name) {
	this->name = name;
}

const char * EndpointQueue::getName() {
	return name;
}

void* EndpointQueue::getPoa() {
	return thePoa;
}
