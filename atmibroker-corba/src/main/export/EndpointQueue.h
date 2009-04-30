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

// Class: EndpointQueue
// A POA servant which implements of the AtmiBroker::ClientCallback interface
//

#ifndef EndpointQueue_H_
#define EndpointQueue_H_

#include "atmiBrokerCorbaMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerS.h"
#endif

#include <queue>
#include "log4cxx/logger.h"
#include "CorbaConnection.h"
#include "Destination.h"
#include "SynchronizableObject.h"

class BLACKTIE_CORBA_DLL EndpointQueue: public virtual Destination, public virtual POA_AtmiBroker::EndpointQueue {
public:
	EndpointQueue(CORBA_CONNECTION* connection);
	EndpointQueue(CORBA_CONNECTION* connection, PortableServer::POA_ptr poa, char* serviceName);
	virtual ~EndpointQueue();

	virtual void send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException );

	virtual void disconnect() throw (CORBA::SystemException );

	virtual MESSAGE receive(long time);

	virtual const char* getName();

	PortableServer::POA_ptr getPoa();

private:
	static log4cxx::LoggerPtr logger;
	std::queue<MESSAGE> returnData;
	SynchronizableObject* lock;
	bool shutdown;
	const char* name;
	PortableServer::POA_ptr thePoa;

	// The following are not implemented
	EndpointQueue(const EndpointQueue &);
	EndpointQueue& operator=(const EndpointQueue &);
};

#endif
