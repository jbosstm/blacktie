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

#ifdef TAO_COMP
#include "tao/ORB.h"
#include "CosTransactionsS.h"
#elif ORBIX_COMP
#include <omg/CosTransactions.hh>
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include "CosTransactions_s.hh"
#include <orb.h>
#endif

#include "atmiBrokerMacro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "AtmiBrokerBuffers.h"
#include "AtmiBrokerServiceRetrieve.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerConversation.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "xatmi.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerConversation(Logger::getLogger("AtmiBrokerConversation"));

AtmiBrokerConversation *AtmiBrokerConversation::ptrAtmiBrokerConversation = NULL;

AtmiBrokerConversation *
AtmiBrokerConversation::get_instance() {
	if (ptrAtmiBrokerConversation == NULL)
		ptrAtmiBrokerConversation = new AtmiBrokerConversation(::ptrAtmiBrokerClient);
	return ptrAtmiBrokerConversation;
}

void AtmiBrokerConversation::discard_instance() {
	if (ptrAtmiBrokerConversation != NULL) {
		delete ptrAtmiBrokerConversation;
		ptrAtmiBrokerConversation = NULL;
	}
}

AtmiBrokerConversation::AtmiBrokerConversation(AtmiBrokerClient* aAtmiBrokerClient) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "constructor");
	mAtmiBrokerClient = aAtmiBrokerClient;
}

AtmiBrokerConversation::~AtmiBrokerConversation() {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "destructor");
}

int AtmiBrokerConversation::tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpcall - svc: %s idata: %s ilen: %d flags: %d", svc, idata, ilen, flags);

	AtmiBroker::octetSeq * a_idata = NULL;
	CORBA::Long a_ilen = ilen;
	AtmiBroker::octetSeq * a_odata = NULL;
	AtmiBroker::TypedBuffer * a_oTypedBufferdata;
	CORBA::Long a_olen = 0;
	CORBA::Long a_flags = flags;
	CORBA::Short a_result = 0;

	AtmiBroker::Service_var aCorbaService;

	char type[25];
	strcpy(type, "");
	char subtype[25];
	strcpy(subtype, "");
	long atype = AtmiBrokerMem::get_instance()->tptypes(idata, type, subtype);
	if (atype == -1L) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "MEMORY NOT ALLOCATED THRU TPALLOC!!!");
		return -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "type of memory: '%s'  subtype: '%s'", type, subtype);
	}

	int status = -1;
	char *id = (char*) malloc(sizeof(char*) * XATMI_SERVICE_NAME_LENGTH);

	try {
		mAtmiBrokerClient->getService(svc, false, &id, &aCorbaService);
	} catch (CORBA::Exception &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "tpcall Could not lookup service '%s' %s", svc, ex._name());
		tperrno = TPENOENT;
		return -1;
	}

	if (CORBA::is_nil(aCorbaService))
		return status;

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object id is %s", id);
	char * aStr = client_orb->object_to_string(aCorbaService);

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
#ifdef TAO_COMP
	CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
#else
	//#elif ORBIX_COMP
	CORBA::Boolean aBoolean = aCorbaService->_validate_connection(*policyList);
#endif
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d", aBoolean);

	try {
		// execute 'tpcall'


		CurrentImpl* currentImpl = AtmiBrokerOTS::get_instance()->getCurrentImpl();
		CosTransactions::Control_ptr aControlPtr = NULL;
		if (currentImpl == NULL) {
			aControlPtr = CosTransactions::Control::_nil();
		} else {
			aControlPtr = currentImpl->get_control();
		}
		if (strcmp(type, TYPE1) == 0)
			a_result = aCorbaService->service_typed_buffer_request_explicit((AtmiBroker::TypedBuffer&) *idata, a_ilen, a_oTypedBufferdata, a_olen, a_flags, aControlPtr);
		else {
			a_idata = new AtmiBroker::octetSeq(a_ilen, a_ilen, (unsigned char *) idata, true);
			a_result = aCorbaService->service_request_explicit(*a_idata, a_ilen, a_odata, a_olen, a_flags, aControlPtr);
		}
		status = a_result;

		if (a_odata) {
			// TODO set the error code when there is no output
			// populated odata and olen
			*odata = (char*) a_odata->get_buffer();
		} else {
			tperrno = TPESVCERR;
			return -1;
		}

	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->service_request(): call failed. %s", ex._name());
	}
	free(id);
	return status;
}

int AtmiBrokerConversation::tpacall(char * svc, char* idata, long ilen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpacall - svc: %s idata: %s ilen: %d flags: %d", svc, idata, ilen, flags);

	AtmiBroker::octetSeq * a_idata;
	CORBA::Long a_ilen = ilen;

	AtmiBroker::Service_var aCorbaService;

	char type[25];
	strcpy(type, "");
	char subtype[25];
	strcpy(subtype, "");
	long atype = AtmiBrokerMem::get_instance()->tptypes(idata, type, subtype);
	if (atype == -1L) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "MEMORY NOT ALLOCATED THRU TPALLOC!!!");
		return -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "type of memory: '%s'  subtype: '%s'", type, subtype);
	}

	int status = -1;
	char *id = (char*) malloc(sizeof(char*) * XATMI_SERVICE_NAME_LENGTH);

	//CORBA::Boolean 	conversation = false;  	// false uses Callback
	CORBA::Boolean conversation = true; // true use tpgetrply

	mAtmiBrokerClient->getService(svc, conversation, &id, &aCorbaService);
	if (CORBA::is_nil(aCorbaService))
		return status;

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object id is %s", id);

	int idInt = mAtmiBrokerClient->convertIdToInt(id);
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object int id is %d", idInt);

	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
	//CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d" aBoolean);

	try {
		// execute 'tpacall'
		if (strcmp(type, TYPE1) == 0)
			aCorbaService->service_typed_buffer_request_async((AtmiBroker::TypedBuffer&) *idata, a_ilen, flags);
		else {

			a_idata = new AtmiBroker::octetSeq(a_ilen, a_ilen, (unsigned char *) idata, true);
			aCorbaService->service_request_async(*a_idata, a_ilen, flags);
		}
	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->service_request_async(): call failed. %s", ex._name());
	}
	free(id);
	return idInt;
}

int AtmiBrokerConversation::tpconnect(char * svc, char* idata, long ilen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpconnect - svc: %s idata: %s ilen: %d flags: %d", svc, idata, ilen, flags);

	AtmiBroker::octetSeq * a_idata;
	CORBA::Long a_ilen = ilen;
	AtmiBroker::octetSeq * a_odata;
	AtmiBroker::TypedBuffer * a_oTypedBufferdata;
	CORBA::Long a_olen = 0;
	CORBA::Long a_flags = flags;
	CORBA::Short a_result;

	AtmiBroker::Service_var aCorbaService;

	char type[25];
	strcpy(type, "");
	char subtype[25];
	strcpy(subtype, "");
	long atype = AtmiBrokerMem::get_instance()->tptypes(idata, type, subtype);
	if (atype == -1L) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "MEMORY NOT ALLOCATED THRU TPALLOC!!!");
		return -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "type of memory: '%s'  subtype: '%s'", type, subtype);
	}

	int status = -1;
	char *id = (char*) malloc(sizeof(char*) * XATMI_SERVICE_NAME_LENGTH);

	mAtmiBrokerClient->getService(svc, true, &id, &aCorbaService);
	if (CORBA::is_nil(aCorbaService))
		return status;

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object id is %s", id);

	int idInt = mAtmiBrokerClient->convertIdToInt(id);
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object int id is %d", idInt);

	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
	//CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d" aBoolean);

	try {
		// execute 'tpconnect'

		CurrentImpl * currentImpl = AtmiBrokerOTS::get_instance()->getCurrentImpl();
		CosTransactions::Control_ptr aControlPtr = NULL;
		if (currentImpl == NULL) {
			aControlPtr = CosTransactions::Control::_nil();
		} else {
			aControlPtr = currentImpl->get_control();
		}
		if (strcmp(type, TYPE1) == 0)
			a_result = aCorbaService->service_typed_buffer_request_explicit((AtmiBroker::TypedBuffer&) *idata, a_ilen, a_oTypedBufferdata, a_olen, a_flags, aControlPtr);
		else {
			a_idata = new AtmiBroker::octetSeq(a_ilen, a_ilen, (unsigned char *) idata, true);
			a_result = aCorbaService->service_request_explicit(*a_idata, a_ilen, a_odata, a_olen, a_flags, aControlPtr);
		}
		status = a_result;
	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->service_request(): call failed. %s", ex._name());
	}
	free(id);
	return idInt;
}

int AtmiBrokerConversation::tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpsend - id: %d idata: %s ilen: %d flags: %d", id, idata, ilen, flags);

	AtmiBroker::octetSeq * a_idata;
	CORBA::Long a_ilen = ilen;
	AtmiBroker::octetSeq * a_odata;
	AtmiBroker::TypedBuffer * a_oTypedBufferdata;
	CORBA::Long a_olen = 0;
	CORBA::Long a_flags = flags;
	CORBA::Short a_result;

	AtmiBroker::Service_var aCorbaService;

	char type[25];
	strcpy(type, "");
	char subtype[25];
	strcpy(subtype, "");
	long atype = AtmiBrokerMem::get_instance()->tptypes(idata, type, subtype);
	if (atype == -1L) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "MEMORY NOT ALLOCATED THRU TPALLOC!!!");
		return -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "type of memory: '%s'  subtype: '%s'", type, subtype);
	}

	int status = -1;

	char * idStr = mAtmiBrokerClient->convertIdToString(id);
	if (idStr == NULL) {
		// TODO
		return -1;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object string id is %s", idStr);

	mAtmiBrokerClient->findService(idStr, &aCorbaService);
	if (CORBA::is_nil(aCorbaService))
		return status;

	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
	//CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d" aBoolean);

	try {
		// execute 'tpsend'

		CurrentImpl * currentImpl = AtmiBrokerOTS::get_instance()->getCurrentImpl();
		CosTransactions::Control_ptr aControlPtr = NULL;
		if (currentImpl == NULL) {
			aControlPtr = CosTransactions::Control::_nil();
		} else {
			aControlPtr = currentImpl->get_control();
		}
		if (strcmp(type, TYPE1) == 0)
			a_result = aCorbaService->service_typed_buffer_request_explicit((AtmiBroker::TypedBuffer&) *idata, a_ilen, a_oTypedBufferdata, a_olen, a_flags, aControlPtr);
		else {
			a_idata = new AtmiBroker::octetSeq(a_ilen, a_ilen, (unsigned char *) idata, true);
			a_result = aCorbaService->service_request_explicit(*a_idata, a_ilen, a_odata, a_olen, a_flags, aControlPtr);
		}
		status = a_result;

		if (a_odata) {
			// populated odata and olen
			char * odata = (char*) a_odata->get_buffer();
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "odata %s", odata);
		}
		//TODO IS THIS AN ERROR CONDITION?
	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->service_request(): call failed. %s", ex._name());
	}
	free(idStr);
	return status;
}

int AtmiBrokerConversation::tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tprecv - id: %d odata: %s olen: %p flags: %d", id, *odata, olen, flags);

	AtmiBroker::octetSeq * a_odata;
	AtmiBroker::TypedBuffer * a_oTypedBufferData;
	CORBA::Long a_olen = 0;
	CORBA::Long a_flags = flags;
	CORBA::Long a_oevent;
	CORBA::Short a_result;

	AtmiBroker::Service_var aCorbaService;

	int status = -1;

	char * idStr = mAtmiBrokerClient->convertIdToString(id);
	if (idStr == NULL) {
		// TODO
		return -1;
	}

	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object string id is %s", idStr);

	mAtmiBrokerClient->findService(idStr, &aCorbaService);
	if (CORBA::is_nil(aCorbaService))
		return status;

	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
	//CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d" aBoolean);

	try {
		// execute 'tprecv'

		a_result = aCorbaService->service_response(a_odata, a_olen, a_flags, a_oevent);
		status = a_result;

		if (status != -1) {
			// populated odata and olen
			*odata = (char*) a_odata->get_buffer();
			*olen = a_olen;
			*event = a_oevent;
		}

	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->service_response(): call failed. %s", ex._name());

		// TODO WHY IS THIS DONE?
		a_result = aCorbaService->service_typed_buffer_response(a_oTypedBufferData, a_olen, a_flags, a_oevent);
		status = a_result;

		// populated odata and olen
		*odata = (char*) a_odata->get_buffer();
		*olen = a_olen;
		*event = a_oevent;

	}
	free(idStr);
	return status;
}

/*
 int
 tpgetrply (int *idPtr, char ** odata, long *olen, long flags)
 {
 userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpgetrply - id: %d odata: %s olen: %p flags: %d", *idPtr, *odata, olen, flags);

 long events;
 int status =  tprecv(*idPtr, odata, olen, flags, &events);
 tpdiscon(*idPtr);
 return status;
 }
 */

int AtmiBrokerConversation::tpdiscon(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpdiscon - id: %d", id);

	AtmiBroker::ServiceFactory_var aCorbaServiceFactory;

	char index[5];

	char *serviceName = (char*) malloc(sizeof(char) * XATMI_SERVICE_NAME_LENGTH);

	int status = -1;

	char * idStr = mAtmiBrokerClient->convertIdToString(id);
	if (idStr == NULL) {
		// TODO
		return -1;
	}

	mAtmiBrokerClient->extractServiceAndIndex(idStr, serviceName, index);

	aCorbaServiceFactory = get_service_factory(serviceName);
	if (CORBA::is_nil(aCorbaServiceFactory))
		return status;

	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validating connection ");
	//CORBA::Boolean aBoolean = aCorbaService->_validate_connection(policyList);
	//userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "validated connection %d" aBoolean);

	try {
		// execute 'tpdiscon'

		aCorbaServiceFactory->end_conversation(mAtmiBrokerClient->getClientId(serviceName), index);
		status = 1;
	} catch (const CORBA::SystemException &ex) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->end_conversation(): call failed. %s", ex._name());
		status = -1;
	}
	free(idStr);
	free(serviceName);
	return status;
}

int AtmiBrokerConversation::tpcancel(int id) {
	//TODO THIS IS JUST A PASS THRU
	userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "tpcancel - id: %d", id);
	return -1;
	//return tpdiscon(id);
}
