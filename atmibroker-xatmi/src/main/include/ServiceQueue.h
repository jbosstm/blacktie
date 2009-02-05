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

// Class: ServiceQueue
// A POA servant which implements of the AtmiBroker::ServiceFactory interface
//

#ifndef ServiceQueue_H_
#define ServiceQueue_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerS.h"
#endif

#include <vector>
#include <queue>
#include "SynchronizableObject.h"
#include "xatmi.h"
#include "ServiceDispatcher.h"
#include "Message.h"
#include "Destination.h"
#include "AtmiBrokerServiceXml.h"

class ATMIBROKER_DLL ServiceQueue: public virtual Destination, public virtual POA_AtmiBroker::EndpointQueue {
public:
	ServiceQueue(void* thePoa, char *serviceName, void(*func)(TPSVCINFO *));
	virtual ~ServiceQueue();

	virtual void send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException );

	virtual void disconnect();

	void* getPoa();

	SVCINFO get_service_info();

	virtual void send(MESSAGE message);

	virtual MESSAGE receive(long flags);

	virtual const char* getName();
protected:
	void* thePoa;
	char* serviceName;
	bool m_shutdown;
	SynchronizableObject* lock;
	std::queue<MESSAGE> messageQueue;
	std::vector<ServiceDispatcher *> servantVector;
	SVCINFO serviceInfo;

private:
	// The following are not implemented
	//
	ServiceQueue(const ServiceQueue &);
	ServiceQueue& operator=(const ServiceQueue &);
};

#endif
