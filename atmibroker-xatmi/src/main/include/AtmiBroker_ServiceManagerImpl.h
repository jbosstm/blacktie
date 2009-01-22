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

// Class: AtmiBroker_ServiceManagerImpl
// A POA servant which implements of the AtmiBroker::ServiceManager interface
//

#ifndef ATMIBROKER_SERVICEMANAGERIMPL_H_
#define ATMIBROKER_SERVICEMANAGERIMPL_H_

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

#include "it_servant_base_overrides.h"

class ATMIBROKER_DLL AtmiBroker_ServiceManagerImpl: public virtual IT_ServantBaseOverrides, public virtual POA_AtmiBroker::ServiceManager {
public:
	AtmiBroker_ServiceManagerImpl(PortableServer::POA_ptr, char *serviceName);

	virtual ~AtmiBroker_ServiceManagerImpl();

	// _create() -- create a new servant.
	// Hides the difference between direct inheritance and tie servants.
	//
	static POA_AtmiBroker::ServiceManager*
	_create(PortableServer::POA_ptr, char *serviceName);

	// IDL operations
	//
	virtual void send_data(const char* ior, CORBA::Boolean inConversation, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CORBA::Long revent) throw (CORBA::SystemException );

	// IDL attributes
	//
	virtual char* serviceName() throw (CORBA::SystemException);

protected:
	// Instance variables for attributes.
	//
	CORBA::String_var m_serviceName;

private:
	// The following are not implemented
	//
	AtmiBroker_ServiceManagerImpl(const AtmiBroker_ServiceManagerImpl &);
	AtmiBroker_ServiceManagerImpl& operator=(const AtmiBroker_ServiceManagerImpl &);
};

#endif
