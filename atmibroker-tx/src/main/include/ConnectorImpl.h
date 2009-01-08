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

#ifndef ConnectorImpl_h
#define ConnectorImpl_h

#include "atmiBrokerTxMacro.h"

#ifdef TAO_COMP
#include "XAC.h"
#elif ORBIX_COMP
#include "XA.hh"
#endif
#ifdef VBC_COMP
#include "XA_c.hh"
#endif

#include "LocalResourceManager.h"
#include "CurrentConnectionImpl.h"

#include <deque>
#include <string>

struct _connectorDataStruct {
	CurrentConnectionImpl* currentConnectionImpl;
	LocalResourceManager* localResourceManager;
};
typedef _connectorDataStruct ConnectorDataStruct;

/**
 "local obj" Implementation of XA::Connector interface.
 this class inherits from the stubs, not the skeletons
 because the XA::Connector interface is a "locality contrained" interface
 */
class ATMIBROKER_TX_DLL ConnectorImpl: public virtual XA::Connector {
public:
	ConnectorImpl();

	virtual ~ConnectorImpl(void);

	ConnectorImpl(const ConnectorImpl &src);
	ConnectorImpl& operator=(const ConnectorImpl &);
	bool operator==(const ConnectorImpl&);

public:
#ifdef TAO_COMP
	virtual XA::ResourceManager_ptr create_resource_manager (const char* resource_manager_name,
			//										XA::XASwitch xa_switch,
			XA::XASwitch_ptr xa_switch,
			const char* open_string,
			const char* close_string,
			XA::ThreadModel thread_model,
			CORBA::Boolean automatic_association,
			CORBA::Boolean dynamic_registration_optimization,
			XA::CurrentConnection_out current_connection)
#else
	//#elif ORBIX_COMP
	virtual XA::ResourceManager_ptr create_resource_manager(const char* resource_manager_name, const XA::XASwitch& xa_switch, const char* open_string, const char* close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization, XA::CurrentConnection_out current_connection)
#endif
			throw(CORBA::SystemException);

#ifdef TAO_COMP
	virtual XA::CurrentConnection_ptr connect_to_resource_manager (XA::ResourceManager_ptr rm,
			//        									XA::XASwitch xa_switch,
			XA::XASwitch_ptr xa_switch,
			const char* open_string,
			const char* close_string,
			XA::ThreadModel thread_model,
			CORBA::Boolean automatic_association,
			CORBA::Boolean dynamic_registration_optimization)
#else
	//#elif ORBIX_COMP
	virtual XA::CurrentConnection_ptr connect_to_resource_manager(XA::ResourceManager_ptr rm, const XA::XASwitch& xa_switch, const char* open_string, const char* close_string, XA::ThreadModel thread_model, CORBA::Boolean automatic_association, CORBA::Boolean dynamic_registration_optimization)
#endif
			throw(CORBA::SystemException);

private:
	std::deque<ConnectorDataStruct*> connectorDataQueue;
};
#endif
