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
EndpointQueue::EndpointQueue(CONNECTION* connection) {
	shutdown = false;
	lock = SynchronizableObject::create(false);

	PortableServer::POA_ptr poa = (PortableServer::POA_ptr) connection->callback_poa;
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "tmp_servant " << this);
	poa->activate_object(this);
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "activated tmp_servant " << this);
	CORBA::Object_ptr tmp_ref = poa->servant_to_reference(this);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	setName(orb->object_to_string(queue));
}

EndpointQueue::EndpointQueue(CONNECTION* connection, void* poa, char* serviceName) {
	shutdown = false;
	thePoa = poa;
	PortableServer::POA_ptr aFactoryPoaPtr = (PortableServer::POA_ptr) poa;
	lock = SynchronizableObject::create(false);
	aFactoryPoaPtr->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_var tmp_ref = aFactoryPoaPtr->servant_to_reference(this);
	CosNaming::Name * name = ((CosNaming::NamingContextExt_ptr) connection->default_ctx)->to_name(serviceName);
	((CosNaming::NamingContext_ptr) connection->name_ctx)->bind(*name, tmp_ref);
	setName(NULL);
}

EndpointQueue::EndpointQueue(CONNECTION* connection, char * callback_ior) {
	CORBA::ORB_ptr orb = (CORBA::ORB_ptr) connection->orbRef;
	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << callback_ior);
	CORBA::Object_var tmp_ref = orb->string_to_object(callback_ior);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "connected to %s" << callback_ior);
}

EndpointQueue::EndpointQueue(CONNECTION* connection, const char * serviceName) {
	CosNaming::NamingContextExt_ptr context = (CosNaming::NamingContextExt_ptr) connection->default_ctx;
	CosNaming::NamingContext_ptr name_context = (CosNaming::NamingContext_ptr) connection->name_ctx;
	LOG4CXX_DEBUG(logger, (char*) "EndpointQueue: " << serviceName);
	CosNaming::Name * name = context->to_name(serviceName);
	CORBA::Object_var tmp_ref = name_context->resolve(*name);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "connected to " << serviceName);
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
	LOG4CXX_DEBUG(logger, (char*) "client_callback(): called.");
	LOG4CXX_DEBUG(logger, (char*) "client_callback():    idata = " << idata.get_buffer());
	LOG4CXX_DEBUG(logger, (char*) "client_callback():    ilen = %d" << ilen);
	LOG4CXX_DEBUG(logger, (char*) "client_callback():    flags = %d" << flags);

	MESSAGE message;
	message.correlationId = correlationId;
	message.data = (char*) malloc(sizeof(char*) * ilen);
	memcpy(message.data, (char*) idata.get_buffer(), ilen);
	message.flags = flags;
	message.len = ilen;
	message.rcode = rcode;
	message.replyto = replyto_ior;
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
	LOG4CXX_ERROR(logger, (char*) "disconnect unimplemented");
	if (remoteEndpoint) {
		remoteEndpoint->disconnect();
	} else {
		lock->lock();
		shutdown = true;
		lock->notify();
		lock->unlock();
	}
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
