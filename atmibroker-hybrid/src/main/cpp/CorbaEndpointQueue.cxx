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
#include "txx.h"
#include "SessionImpl.h"

log4cxx::LoggerPtr HybridCorbaEndpointQueue::logger(log4cxx::Logger::getLogger(
		"HybridCorbaEndpointQueue"));

long TPFAIL = 0x00000001;
long DISCON = 0x00000003;

int TPESVCERR = 10;
int TPESVCFAIL = 11;

long TPEV_DISCONIMM = 0x0001;
long TPEV_SVCERR = 0x0002;
long TPEV_SVCFAIL = 0x0004;

// EndpointQueue constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
HybridCorbaEndpointQueue::HybridCorbaEndpointQueue(HybridSessionImpl* session,
		CORBA_CONNECTION* connection, char* id) {
	LOG4CXX_DEBUG(logger, (char*) "Creating corba endpoint queue");
	shutdown = false;
	lock = new SynchronizableObject();

	CORBA::PolicyList policies;
	policies.length(0);
	thePoa = connection->callback_poa->create_POA(id,
			connection->root_poa_manager, policies);

	LOG4CXX_DEBUG(logger, (char*) "tmp_servant " << this);
	oid = thePoa->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_var tmp_ref = thePoa->servant_to_reference(this);
	AtmiBroker::EndpointQueue_var queue = AtmiBroker::EndpointQueue::_narrow(
			tmp_ref);

	CORBA::ORB_ptr orb = connection->orbRef;
	this->name = orb->object_to_string(queue);
	this->connection = connection;
	this->session = session;
}

HybridCorbaEndpointQueue::HybridCorbaEndpointQueue(
		CORBA_CONNECTION* connection, PortableServer::POA_ptr poa,
		char* serviceName) {
	shutdown = false;
	thePoa = poa;
	lock = new SynchronizableObject();
	thePoa->activate_object(this);
	LOG4CXX_DEBUG(logger, (char*) "activated tmp_servant " << this);
	CORBA::Object_var tmp_ref = thePoa->servant_to_reference(this);
	CosNaming::Name * name = connection->default_ctx->to_name(serviceName);
	connection->name_ctx->bind(*name, tmp_ref);
	this->name = serviceName;
	this->session = NULL;
}

// ~EndpointQueue destructor.
//
HybridCorbaEndpointQueue::~HybridCorbaEndpointQueue() {
	LOG4CXX_DEBUG(logger, (char*) "destroy called: " << this);

	LOG4CXX_DEBUG(logger, (char*) "destroying thePoa: " << thePoa);
	thePoa->destroy(true, true);
	thePoa = NULL;
	LOG4CXX_DEBUG(logger, (char*) "destroyed thePoa: " << thePoa);

	lock->lock();
	while (returnData.size() > 0) {
		MESSAGE message = returnData.front();
		returnData.pop();
		LOG4CXX_DEBUG(logger, (char*) "Freeing data message");
		free(message.data);
		free(message.type);
		free(message.subtype);
		free((char*) message.replyto);
	}
	if (!shutdown) {
		shutdown = true;
		lock->notifyAll();
	}
	lock->unlock();

	delete[] name;
	delete lock;
	LOG4CXX_DEBUG(logger, (char*) "destroyed: " << this);
}

void HybridCorbaEndpointQueue::send(const char* replyto_ior, CORBA::Short rval,
		CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen,
		CORBA::Long correlationId, CORBA::Long flags, const char* type,
		const char* subtype) throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(logger, (char*) "send called:" << this);
	if (!shutdown) {
		lock->lock();
		if (!shutdown) {
			LOG4CXX_DEBUG(logger, (char*) "send called.");
			if (replyto_ior != NULL) {
				LOG4CXX_DEBUG(logger, (char*) "send reply to = " << replyto_ior);
			}
			LOG4CXX_DEBUG(logger, (char*) "send ilen = " << ilen);
			LOG4CXX_DEBUG(logger, (char*) "send flags = " << flags);

			MESSAGE message;
			message.correlationId = correlationId;
			message.flags = flags;
			message.rcode = rcode;
			if (replyto_ior != NULL) {
				LOG4CXX_TRACE(logger, (char*) "Duplicating the replyto");
				message.replyto = strdup(replyto_ior);
				LOG4CXX_TRACE(logger, (char*) "Duplicated");
			} else {
				message.replyto = NULL;
			}
			message.type = strdup(type);
			message.subtype = strdup(subtype);

			message.len = ilen - 1;
			if (message.len == 0 && strlen(message.type) == 0) {
				message.data = NULL;
			} else {
				LOG4CXX_TRACE(logger, (char*) "Allocating DATA");
				message.data = (char*) malloc(message.len);
				LOG4CXX_TRACE(logger, (char*) "Allocated");
				if (message.len > 0) {
					memcpy(message.data, (char*) idata.get_buffer(),
							message.len);
					LOG4CXX_TRACE(logger, (char*) "Copied");
				}
			}

			message.rval = rval;
/* TODO 
			LOG4CXX_TRACE(logger, (char*) "Getting control via disassociate_tx");
			message.control = txx_unbind();
			LOG4CXX_TRACE(logger, (char*) "Got control");
*/

			message.received = true;
			// For remote comms this thread (comes from a pool) is different from the thread that will
			// eventually consume the message. For local comms this is not the case.
			// Thus we cannot dissassociate any transaction from the thread here (using destroySpecific)

			if (message.rval == DISCON) {
				session->setLastEvent(TPEV_DISCONIMM);
			} else if (message.rcode == TPESVCERR) {
				session->setLastEvent(TPEV_SVCERR);
			} else if (message.rval == TPFAIL) {
				session->setLastEvent(TPEV_SVCFAIL);
				session->setLastRCode(message.rcode);
			}
			returnData.push(message);
			LOG4CXX_DEBUG(logger, (char*) "notifying");
			lock->notify();
		}
		LOG4CXX_DEBUG(logger, (char*) "notified");
		lock->unlock();
	}
}

MESSAGE HybridCorbaEndpointQueue::receive(long time) {
	LOG4CXX_DEBUG(logger, (char*) "service_response(): " << time);

	MESSAGE message;
	message.replyto = NULL;
	message.correlationId = -1;
	message.data = NULL;
	message.len = -1;
	message.flags = -1;
	message.control = NULL;
	message.rval = -1;
	message.rcode = -1;
	message.type = NULL;
	message.subtype = NULL;
	message.received = false;
	message.ttl = -1;
	message.serviceName = NULL;

	lock->lock();
	if (!shutdown) {
		if (returnData.size() == 0 && time != 0) {
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

void HybridCorbaEndpointQueue::disconnect() throw (CORBA::SystemException ) {
	LOG4CXX_DEBUG(logger, (char*) "disconnect");
	lock->lock();
	if (!shutdown) {
		shutdown = true;
		lock->notifyAll();
	}
	lock->unlock();
}

const char * HybridCorbaEndpointQueue::getName() {
	return name;
}

PortableServer::POA_ptr HybridCorbaEndpointQueue::getPoa() {
	return thePoa;
}
