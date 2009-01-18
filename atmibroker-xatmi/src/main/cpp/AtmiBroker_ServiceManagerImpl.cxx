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
// Servant which implements the AtmiBroker::ServiceManager interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include <stdlib.h>
#include <iostream>
#include "AtmiBroker_ServiceManagerImpl.h"

#include "AtmiBrokerServer.h"
#include "userlog.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ServiceManagerImpl(Logger::getLogger("AtmiBroker_ServiceManagerImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::ServiceManager*
AtmiBroker_ServiceManagerImpl::_create(PortableServer::POA_ptr the_poa, char *serviceName) {
	return new AtmiBroker_ServiceManagerImpl(the_poa, serviceName);
}

// AtmiBroker_ServiceManagerImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ServiceManagerImpl::AtmiBroker_ServiceManagerImpl(PortableServer::POA_ptr the_poa, char *serviceName) :
	IT_ServantBaseOverrides(the_poa), m_serviceName(serviceName) {
	// Initialise instance variables used for attributes
}

// ~AtmiBroker_ServiceManagerImpl destructor.
//
AtmiBroker_ServiceManagerImpl::~AtmiBroker_ServiceManagerImpl() {
	// Intentionally empty.
	//
}

// service_request_async() -- Implements IDL operation "AtmiBroker::ServiceManager::service_request_async".
//
void AtmiBroker_ServiceManagerImpl::send_data(const char* ior, CORBA::Boolean inConversation, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, CORBA::Long revent, CosTransactions::Control_ptr control) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "service_request_async() ior: %s ", ior);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "string_to_object ");
	CORBA::Object_var tmp_ref = server_orb->string_to_object(ior);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "tmp_ref %d", (void*) tmp_ref);

	AtmiBroker::Service_var aItrPtr = AtmiBroker::Service::_narrow(tmp_ref);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "aItrPtr %d", (void*) aItrPtr);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "calling service_request_async on iterator %d", (void*) aItrPtr);
	aItrPtr->send_data(inConversation, idata, ilen, flags, revent, control);

}

// serviceName() -- Accessor for IDL attribute "AtmiBroker::ServiceManager::serviceName".
//
char*
AtmiBroker_ServiceManagerImpl::serviceName() throw (CORBA::SystemException) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceManagerImpl, (char*) "serviceName() %s", m_serviceName);
	return strdup((char*) m_serviceName);
}

