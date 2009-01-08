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
#include "AtmiBroker_ClientCallbackImpl.h"

#include "AtmiBrokerClient.h"
#include "userlog.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ClientCallbackImpl(Logger::getLogger("AtmiBroker_ClientCallbackImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::ClientCallback*
AtmiBroker_ClientCallbackImpl::_create(PortableServer::POA_ptr the_poa) {
	return new AtmiBroker_ClientCallbackImpl(the_poa);
}

// AtmiBroker_ClientCallbackImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ClientCallbackImpl::AtmiBroker_ClientCallbackImpl(PortableServer::POA_ptr the_poa) :
	IT_ServantBaseOverrides(the_poa) {
	callbackFunctionPtr = NULL;
	callbackTypedBufferFunctionPtr = NULL;
}

// ~AtmiBroker_ClientCallbackImpl destructor.
//
AtmiBroker_ClientCallbackImpl::~AtmiBroker_ClientCallbackImpl() {
	// Intentionally empty.
	//
}

// client_callback() -- Implements IDL operation "AtmiBroker::ClientCallback::client_callback".
//
void AtmiBroker_ClientCallbackImpl::client_callback(const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags, const char * id) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback(): called.");

	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    idata = %s", idata.get_buffer());
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    flags = %d", flags);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():    id = %s", id);

	if (callbackFunctionPtr != NULL) {
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():calling application callback");
		(*callbackFunctionPtr)(idata, ilen, flags, id);
		userlog(Level::getInfo(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_callback():called application callback");
	} else {
		userlog(Level::getError(), loggerAtmiBroker_ClientCallbackImpl, (char*) "Unhandled response");
	}
}

// client_typed_buffer_callback() -- Implements IDL operation "AtmiBroker::ClientCallback::client_typed_buffer_callback".
//
void AtmiBroker_ClientCallbackImpl::client_typed_buffer_callback(const AtmiBroker::TypedBuffer& idata, CORBA::Long ilen, CORBA::Long flags, const char * id) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback(): called.");

	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():    idata = %s", (const char *) idata.name);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():    flags = %d", flags);
	userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():    id = %s", id);

	if (callbackTypedBufferFunctionPtr != NULL) {
		userlog(Level::getDebug(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():calling application ptr");
		(*callbackTypedBufferFunctionPtr)(idata, ilen, flags, id);
		userlog(Level::getInfo(), loggerAtmiBroker_ClientCallbackImpl, (char*) "client_typed_buffer_callback():called application ptr");
	} else {
		userlog(Level::getError(), loggerAtmiBroker_ClientCallbackImpl, (char*) "Unhandled response");
	}

}

