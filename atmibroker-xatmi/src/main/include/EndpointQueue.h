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

//-----------------------------------------------------------------------------
// Edit the idlgen.cfg to have your own copyright notice placed here.
//-----------------------------------------------------------------------------

// Class: EndpointQueue
// A POA servant which implements of the AtmiBroker::ClientCallback interface
//

#ifndef EndpointQueue_H_
#define EndpointQueue_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerS.h"
#endif

#include <queue>
#include "log4cxx/logger.h"
#include "Connection.h"
#include "Destination.h"
#include "SynchronizableObject.h"

class ATMIBROKER_DLL EndpointQueue: public virtual Destination, public virtual POA_AtmiBroker::EndpointQueue {
public:
	EndpointQueue(CONNECTION* connection);
	EndpointQueue(CONNECTION* connection, void* poa, char* serviceName);
	EndpointQueue(CONNECTION* connection, char * callback_ior);
	EndpointQueue(CONNECTION* connection, const char * serviceName);
	virtual ~EndpointQueue();

	virtual void send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException );

	virtual void disconnect() throw (CORBA::SystemException );

	void setName(const char * name);

	virtual void send(MESSAGE message);

	virtual MESSAGE receive(bool noWait);

	virtual const char* getName();

	void* getPoa();

private:
	static log4cxx::LoggerPtr logger;
	std::queue<MESSAGE> returnData;
	SynchronizableObject* lock;
	bool shutdown;
	const char* name;
	void* thePoa;
	AtmiBroker::EndpointQueue_var remoteEndpoint;

	// The following are not implemented
	EndpointQueue(const EndpointQueue &);
	EndpointQueue& operator=(const EndpointQueue &);
};

#endif
