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
// Servant which implements the AtmiBroker::Service interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include <orb.h>
#include "AtmiBroker_c.hh"
#endif

#include <stdlib.h>
#include <iostream>
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerClient.h"
#include "AtmiBroker.h"

#include "AtmiBrokerOTS.h"
#include "AtmiBrokerBuffers.h"
#include "xatmi.h"
#include "userlog.h"
#include "atmiBrokerMacro.h"
#include "ThreadLocalStorage.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBroker_ServiceImpl(Logger::getLogger("AtmiBroker_ServiceImpl"));

// _create() -- create a new servant.
// Hides the difference between direct inheritance and tie servants
// For direct inheritance, simple create and return an instance of the servant.
// For tie, creates an instance of the tied class and the tie, return the tie.
//
POA_AtmiBroker::Service*
AtmiBroker_ServiceImpl::_create(AtmiBroker_ServiceFactoryImpl* aParent, PortableServer::POA_ptr the_poa, int aIndex, char *serviceName, void(*func)(TPSVCINFO *)) {
	return new AtmiBroker_ServiceImpl(aParent, the_poa, aIndex, serviceName, func);
}

// AtmiBroker_ServiceImpl constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
AtmiBroker_ServiceImpl::AtmiBroker_ServiceImpl(AtmiBroker_ServiceFactoryImpl* aParent, PortableServer::POA_ptr the_poa, int aIndex, char *serviceName, void(*func)(TPSVCINFO *)) :
	IT_ServantBaseOverrides(the_poa), parent(aParent), returnStatus(-1), inConversation(false), inUse(false), callbackRef(NULL), conversationCount(0), index(aIndex), m_serviceName(serviceName), m_func(func) {
	// Initialise instance variables used for attributes
	//
	//TJJ key = getKey();
}

// ~AtmiBroker_ServiceImpl destructor.
//
AtmiBroker_ServiceImpl::~AtmiBroker_ServiceImpl() {
	// Intentionally empty.
	//
}

//CosTransactions::Control::_nil()
// service_request_explicit() -- Implements IDL operation "AtmiBroker::Service::service_request_explicit".
//
CORBA::Short AtmiBroker_ServiceImpl::service_request_explicit(const AtmiBroker::octetSeq& idata, CORBA::Long ilen, AtmiBroker::octetSeq_out odata, CORBA::Long_out olen, CORBA::Long flags, CosTransactions::Control_ptr control) throw (CORBA::SystemException ) {
	dataType = (char*) X_OCTET;
	m_octetSeq = &idata;
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit() dataType: %s", dataType);
	TPSVCINFO tpsvcinfo;

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    idata = %s", idata.get_buffer());
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    flags = %d", flags);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit() about to call createConnectionTransactionAssociation");
	createConnectionTransactionAssociation(control);

	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, m_serviceName);
	tpsvcinfo.flags = flags;
	tpsvcinfo.data = (char*) idata.get_buffer();
	tpsvcinfo.len = ilen;

	setSpecific(1, this);
	m_func(&tpsvcinfo);
	destroySpecific(1);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    octet array of size %d ", returnData.size());
	if (returnData.size() > 0) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    fronting octet array of size %d ", returnData.size());
		AtmiBroker::octetSeq * aOctetSeq = (AtmiBroker::octetSeq *) returnData.front();
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    fronted octet array %s", (char*) aOctetSeq->get_buffer());
		odata = new AtmiBroker::octetSeq(*aOctetSeq);
		olen = odata->length();

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    odata = %s", odata->get_buffer());
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_explicit()    olen = %d", olen);
	}

	// Return normally.
	//
	return returnStatus;
}

// service_typed_buffer_request_explicit() -- Implements IDL operation "AtmiBroker::Service::service_typed_buffer_request_explicit".
//
CORBA::Short AtmiBroker_ServiceImpl::service_typed_buffer_request_explicit(const AtmiBroker::TypedBuffer& idata, CORBA::Long ilen, AtmiBroker::TypedBuffer_out odata, CORBA::Long_out olen, CORBA::Long flags, CosTransactions::Control_ptr control) throw (CORBA::SystemException ) {
	dataType = (char*) TYPE1;
	m_typedBuffer = &idata;
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit() dataType %s", dataType);
	TPSVCINFO tpsvcinfo;

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    idata = %s", (const char *) idata.name);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    flags = %d", flags);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit() about to call createConnectionTransactionAssociation");
	createConnectionTransactionAssociation(control);

	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, m_serviceName);
	tpsvcinfo.flags = flags;
	tpsvcinfo.data = (char*) &idata;
	tpsvcinfo.len = ilen;

	setSpecific(1, this);
	m_func(&tpsvcinfo);
	destroySpecific(1);

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    typed buffer of size %d ", returnTypedBufferData.size());
	if (returnTypedBufferData.size() > 0) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    fronting typed buffer of size %d ", returnTypedBufferData.size());
		AtmiBroker::TypedBuffer * aTypedBuffer = (AtmiBroker::TypedBuffer *) returnTypedBufferData.front();
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    fronted typed buffer %p", aTypedBuffer);
		odata = new AtmiBroker::TypedBuffer(*aTypedBuffer);
		olen = sizeof(odata);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    fronted typed buffer %p", (void*) odata);

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    odata = %p", (void*) odata);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_explicit()    olen = %d", olen);
	}
	// Return normally.
	//
	return returnStatus;
}

// service_request_async() -- Implements IDL operation "AtmiBroker::Service::service_request_async".
//
void AtmiBroker_ServiceImpl::service_request_async(const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long flags) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_async()");

	AtmiBroker::ClientInfo client_info;
	client_info.client_id = clientId;

	char * callback_ior = ptrServer->get_client_callback(client_info);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "client callback_ior for id %d is %s", clientId, callback_ior);

	if (callback_ior != NULL && strcmp(callback_ior, "") != 0) {
		CORBA::Object_var tmp_ref = server_orb->string_to_object(callback_ior);
		callbackRef = AtmiBroker::ClientCallback::_narrow(tmp_ref);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   callback 	= %p", (void*) callbackRef);
	} else
		userlog(Level::getError(), loggerAtmiBroker_ServiceImpl, (char*) "   NO callback");

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   idata = %s", idata.get_buffer());
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   flags = %d", flags);

	AtmiBroker::octetSeq* odata;
	CORBA::Long olen;

	// TODO NO transaction
	CORBA::Short _result = service_request_explicit(idata, ilen, odata, olen, flags, CosTransactions::Control::_nil());

	if (inConversation) {
		returnStatus = _result;
		returnData.push(new AtmiBroker::octetSeq(*odata));
	}

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_request_async(): returning.");
}

// service_typed_buffer_request_async() -- Implements IDL operation "AtmiBroker::Service::service_typed_buffer_request_async".
//
void AtmiBroker_ServiceImpl::service_typed_buffer_request_async(const AtmiBroker::TypedBuffer& idata, CORBA::Long ilen, CORBA::Long flags) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_async()");

	AtmiBroker::ClientInfo client_info;
	client_info.client_id = clientId;

	char * callback_ior = ptrServer->get_client_callback(client_info);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "client callback_ior for id %d is %s", clientId, callback_ior);

	if (callback_ior != NULL && strcmp(callback_ior, "") != 0) {
		CORBA::Object_var tmp_ref = server_orb->string_to_object(callback_ior);
		callbackRef = AtmiBroker::ClientCallback::_narrow(tmp_ref);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   callback 	= %p", (void*) callbackRef);
	} else
		userlog(Level::getError(), loggerAtmiBroker_ServiceImpl, (char*) "   NO callback");

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   idata = %s", (const char*) idata.name);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   ilen = %d", ilen);
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   flags = %d", flags);

	AtmiBroker::TypedBuffer* odata;
	CORBA::Long olen;

	// TODO NO TRANSACTION
	CORBA::Short _result = service_typed_buffer_request_explicit(idata, ilen, odata, olen, flags, CosTransactions::Control::_nil());

	if (inConversation) {
		returnStatus = _result;
		returnTypedBufferData.push(new AtmiBroker::TypedBuffer(*odata));
	}

	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_request_async(): returning.");
}

// service_response() -- Implements IDL operation "AtmiBroker::Service::service_response".
//
CORBA::Short AtmiBroker_ServiceImpl::service_response(AtmiBroker::octetSeq_out odata, CORBA::Long_out olen, CORBA::Long flags, CORBA::Long_out event) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_response() dataType: %s", dataType);

	if (strcmp(dataType, X_OCTET) == 0 || strcmp(dataType, X_C_TYPE) == 0 || strcmp(dataType, X_COMMON) == 0) {
		// TODO THIS SHOULD CHECK THE FLAGS TO WAIT FOR THE DATA.. (on the client-side most likely!)
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "    fronting octet array of size %d ", returnData.size());
		if (returnData.size() > 0) {
			AtmiBroker::octetSeq * aOctetSeq = (AtmiBroker::octetSeq *) returnData.front();
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "    fronted octet array %s", (char*) aOctetSeq->get_buffer());
			odata = new AtmiBroker::octetSeq(*aOctetSeq);
			olen = strlen((char*) odata->get_buffer());

			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   odata = %s", odata->get_buffer());
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   olen  = %d", olen);
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   event = %d", event);

			returnData.pop();

			returnStatus = 1;
			return returnStatus;
		} else {
			return -1;
		}
	}
	return -1;
}

// service_typed_buffer_response() -- Implements IDL operation "AtmiBroker::Service::service_typed_buffer_response".
//
CORBA::Short AtmiBroker_ServiceImpl::service_typed_buffer_response(AtmiBroker::TypedBuffer_out odata, CORBA::Long_out olen, CORBA::Long flags, CORBA::Long_out event) throw (CORBA::SystemException ) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "service_typed_buffer_response() dataType %s", dataType);

	if (strcmp(dataType, TYPE1) == 0 ) {
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "    fronting typed buffer of size %d ", returnTypedBufferData.size());
		AtmiBroker::TypedBuffer * aTypedBuffer = (AtmiBroker::TypedBuffer *) returnTypedBufferData.front();
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "    fronted typed buffer %p", aTypedBuffer);
		odata = new AtmiBroker::TypedBuffer(*aTypedBuffer);
		olen = sizeof(odata);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "    fronted typed buffer %p", (void *) odata);

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   odata = %p", (void*) odata);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   olen  = %d", olen);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "   event = %d", event);

		returnTypedBufferData.pop();

		returnStatus = 1;
		return returnStatus;
	}
	return -1;
}

// serviceName() -- Accessor for IDL attribute "AtmiBroker::Service::serviceName".
//
char*
AtmiBroker_ServiceImpl::serviceName() throw (CORBA::SystemException) {
	return m_serviceName;
}

// mytpreturn()
//
void AtmiBroker_ServiceImpl::mytpreturn(int rval, long rcode, char* data, long len, long flags) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "mytpreturn()");

	returnStatus = rval;
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "dataType: %s", dataType);
	char *idStr = (char*) malloc(sizeof(char) * (XATMI_SERVICE_NAME_LENGTH *2));
	strcpy(idStr, m_serviceName);
	strcat(idStr, ":");
	// ltoa
	std::ostringstream oss;
	oss << index << std::dec;
	const char* indexStr = oss.str().c_str();

	strcat(idStr, indexStr);
	CORBA::String_var id = CORBA::string_dup(idStr);

	endConnectionTransactionAssociation();

	if (strcmp(dataType, X_OCTET) == 0 || strcmp(dataType, X_C_TYPE) == 0 || strcmp(dataType, X_COMMON) == 0) {
		if (!CORBA::is_nil(callbackRef)) //dispatcherThread->
		{
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "Calling back ");
			AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(len, len, (unsigned char *) data, true);
			callbackRef->client_callback(aOctetSeq, len, flags, id); //dispatcherThread->
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "Called back ");
			aOctetSeq = NULL;
		} else {
			AtmiBroker::octetSeq * aOctetSeq = new AtmiBroker::octetSeq(len, len, (unsigned char *) data, true);
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushing octet data %s", (char*) aOctetSeq->get_buffer());
			returnData.push(aOctetSeq);
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushed octet  data %s", (char*) aOctetSeq->get_buffer());
		}
	} else {
		if (!CORBA::is_nil(callbackRef)) {
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "Calling back ");
			AtmiBroker::TypedBuffer_var aTypedBuffer = new AtmiBroker::TypedBuffer((AtmiBroker::TypedBuffer&) *data);
			callbackRef->client_typed_buffer_callback(aTypedBuffer, len, flags, id); //dispatcherThread->
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "Called back ");
		} else {
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushing typed buffer data %p", data);
			AtmiBroker::TypedBuffer * aTypedBuffer = new AtmiBroker::TypedBuffer((AtmiBroker::TypedBuffer&) *data);
			returnTypedBufferData.push(aTypedBuffer); //dispatcherThread->
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushed typed buffer data %p", data);
		}
	}

	inConversation = false; //dispatcherThread->
	conversationCount = 0; //dispatcherThread->
	inUse = false; //dispatcherThread->
	clientId = 0; //dispatcherThread->
	callbackRef = NULL; //dispatcherThread->
	free(idStr);
}

// tpsend()
//
int AtmiBroker_ServiceImpl::tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "tpsend()");

	if (strcmp(dataType, X_OCTET) == 0 || strcmp(dataType, X_C_TYPE) == 0 || strcmp(dataType, X_COMMON) == 0) {
		AtmiBroker::octetSeq * aOctetSeq = new AtmiBroker::octetSeq(ilen, ilen, (unsigned char *) idata, true);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushing octet data %s", (char*) aOctetSeq->get_buffer());
		returnData.push(aOctetSeq);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushed octet  data %s", (char*) aOctetSeq->get_buffer());
	} else {
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushing typed buffer data %p", idata);
		AtmiBroker::TypedBuffer * aTypedBuffer = new AtmiBroker::TypedBuffer((AtmiBroker::TypedBuffer&) *idata);
		returnTypedBufferData.push(aTypedBuffer);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "pushed typed buffer data %p", idata);
	}
	return 1;
}

CORBA::Boolean AtmiBroker_ServiceImpl::isInConversation() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "isInConversation() %d", inConversation);
	return inConversation;
}

void AtmiBroker_ServiceImpl::setInConversation(CORBA::Boolean aInd) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "setInConversation() %d", aInd);
	conversationCount = 0;
	inConversation = aInd;
	for (; returnData.size() > 0;)
		returnData.pop();
	for (; returnTypedBufferData.size() > 0;)
		returnTypedBufferData.pop();
}

CORBA::Boolean AtmiBroker_ServiceImpl::isInUse() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "isInUse() %d", inUse);
	return inUse;
}

void AtmiBroker_ServiceImpl::setInUse(CORBA::Boolean aInd) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "setInUse() %d", aInd);
	inUse = aInd;
}

long AtmiBroker_ServiceImpl::getClientId() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "getClientId() %d", clientId);
	return clientId;
}

void AtmiBroker_ServiceImpl::setClientId(long aClientId) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "setClientId() %d", aClientId);
	clientId = aClientId;
}

void AtmiBroker_ServiceImpl::createConnectionTransactionAssociation(CosTransactions::Control_ptr control) {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "createConnectionTransactionAssociation %p", control);

	try {
		if (CORBA::is_nil(control)) {
			userlog(Level::getError(), loggerAtmiBroker_ServiceImpl, (char*) "NO  TRANSACTION associated with this call");
			return;
		}

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "about to duplicate control: %p", (void*) control);
		tx_control = CosTransactions::Control::_duplicate(control);
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "control: %p", (void*) tx_control);

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "getting Coordinator ");
		tx_coordinator = tx_control->get_coordinator();
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "got Coordinator: %p", (void*) tx_coordinator);

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "getting Propagation Context ");
		tx_propagation_context = tx_coordinator->get_txcontext();
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "got Propagation Context: %p", (void*) tx_propagation_context);

		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "getting Transaction Id ");
		otid = tx_propagation_context->current.otid;
		userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "got Transaction Id: %p", (void*) &otid);

		//		if (!CORBA::is_nil(AtmiBrokerOTS::get_instance()->getXaCurrentConnection()))
		{
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "associating current connection with this transaction ");
			//TODO xa_current_connection->start(tx_coordinator, otid);
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "NOT associating current connection with this transaction until hooked in with Oracle ");
		}
	} catch (CORBA::SystemException &e) {
		userlog(Level::getError(), loggerAtmiBroker_ServiceImpl, (char*) "could not connect transaction %s", (void*) e._name());
	}

}

void AtmiBroker_ServiceImpl::endConnectionTransactionAssociation() {
	userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "endConnectionTransactionAssociation()");

	try {
		//		if (!CORBA::is_nil(AtmiBrokerOTS::get_instance()->getXaCurrentConnection()))
		{
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "disassociating current connection with this transaction ");
			//TODO xa_current_connection->end(tx_coordinator, otid, true);
			userlog(Level::getDebug(), loggerAtmiBroker_ServiceImpl, (char*) "NOT disassociating current connection with this transaction until hooked in with Oracle ");
		}
	} catch (CORBA::TRANSACTION_ROLLEDBACK & aRef) {
		userlog(Level::getError(), loggerAtmiBroker_ServiceImpl, (char*) "transaction has been rolled back %p", (void*) &aRef);
	}
}

bool AtmiBroker_ServiceImpl::sameBuffer(char* toCheck) {
	bool toReturn = false;
	if (m_octetSeq && toCheck == (char*) m_octetSeq->get_buffer()) {
		toReturn = true;
	} else if (m_typedBuffer && toCheck == (char*) &m_typedBuffer) {
		toReturn = true;
	}
	return toReturn;
}

