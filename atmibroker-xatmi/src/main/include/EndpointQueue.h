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
#include "tao/ORB.h"
#include "AtmiBrokerS.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#include "AtmiBrokerS.hh"
#endif
#ifdef VBC_COMP
#include <orb.h>
#include "AtmiBroker_s.hh"
#endif

#include <queue>

#include "Receiver.h"
#include "SynchronizableObject.h"
#include "Message.h"

class ATMIBROKER_DLL EndpointQueue: public virtual Receiver, public virtual POA_AtmiBroker::EndpointQueue {
public:
	EndpointQueue(PortableServer::POA_ptr);

	virtual ~EndpointQueue();

	// _create() -- create a new servant.
	// Hides the difference between direct inheritance and tie servants.
	//
	static POA_AtmiBroker::EndpointQueue* _create(PortableServer::POA_ptr);

	virtual void send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CORBA::Long revent) throw (CORBA::SystemException );

	virtual void disconnect() throw (CORBA::SystemException );

	void setReplyTo(const char * replyTo);

	virtual MESSAGE receive(long flags);

	virtual const char* getDestinationName();

private:
	// The following are not implemented
	//
	EndpointQueue(const EndpointQueue &);
	EndpointQueue& operator=(const EndpointQueue &);
	std::queue<MESSAGE> returnData;
	SynchronizableObject* lock;
	const char * m_replyTo;
};

#endif
