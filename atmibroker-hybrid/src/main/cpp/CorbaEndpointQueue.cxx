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
//
// Servant which implements the AtmiBroker::ClientCallback interface.
//

#ifdef TAO_COMP
#include <orbsvcs/CosNamingS.h>
#endif

#include "CorbaEndpointQueue.h"
#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr CorbaEndpointQueue::logger(log4cxx::Logger::getLogger("CorbaEndpointQueue"));

// EndpointQueue constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
CorbaEndpointQueue::CorbaEndpointQueue(CORBA_CONNECTION* connection) {
	shutdown = false;
	lock = new SynchronizableObject();

	PortableServer::POA_ptr poa = (PortableServer::POA_ptr) connection->callback_poa;
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	LOG4CXX_DEBUG(logger, (char*) "tmp_servant " << this);
	poa->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_ptr tmp_ref = poa->servant_to_reference(this);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	this->name = orb->object_to_string(queue);
}

CorbaEndpointQueue::CorbaEndpointQueue(CORBA_CONNECTION* connection, PortableServer::POA_ptr poa, char* serviceName) {
	shutdown = false;
	thePoa = poa;
	lock = new SynchronizableObject();
	thePoa->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_var tmp_ref = thePoa->servant_to_reference(this);
	CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serviceName);
	((CosNaming::NamingContext_ptr) connection->name_ctx)->bind(*name, tmp_ref);
	this->name = serviceName;
}

// ~EndpointQueue destructor.
//
CorbaEndpointQueue::~CorbaEndpointQueue() {
	LOG4CXX_DEBUG(logger, (char*) "destroy called");

	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notifyAll();
	}
	lock->unlock();
	delete lock;
	lock = NULL;


	LOG4CXX_DEBUG(logger, (char*) "destroying poa: " << thePoa);
	thePoa->destroy(true, true);
	thePoa = NULL;
	LOG4CXX_DEBUG(logger, (char*) "destroyed poa: " << thePoa);

	LOG4CXX_DEBUG(logger, (char*) "destroyed");
}

void CorbaEndpointQueue::send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException ) {
	lock->lock();
	if (!shutdown) {
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
		// For remote comms this thread (comes from a pool) is different from the thread that will
		// eventually consume the message. For local comms this is not the case.
		// Thus we cannot dissassociate any transaction from the thread here (using destroySpecific)

		returnData.push(message);
		LOG4CXX_DEBUG(logger, (char*) "notifying");
		lock->notify();
	}
	LOG4CXX_DEBUG(logger, (char*) "notified");
	lock->unlock();
}

MESSAGE CorbaEndpointQueue::receive(long time) {
	LOG4CXX_DEBUG(logger, (char*) "service_response()");

	MESSAGE message;
	message.replyto = NULL;
	message.correlationId = -1;
	message.data = NULL;
	message.len = -1;
	message.flags = -1;
	message.control = NULL;
	message.rval = -1;
	message.rcode = -1;

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

void CorbaEndpointQueue::disconnect() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(logger, (char*) "disconnect");
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notifyAll();
	}
	lock->unlock();
}

const char * CorbaEndpointQueue::getName() {
	return name;
}

PortableServer::POA_ptr CorbaEndpointQueue::getPoa() {
	return thePoa;
}
