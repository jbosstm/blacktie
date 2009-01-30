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
#include "AtmiBrokerC.h"
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

#include "ThreadLocalStorage.h"
#include "AtmiBrokerBuffers.h"
#include "AtmiBrokerServiceRetrieve.h"
#include "AtmiBrokerOTS.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerConversation.h"
#include "AtmiBrokerPoaFac.h"
#include "userlog.h"
#include "xatmi.h"
#include "tx.h"

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
	int cd = tpacall(svc, idata, ilen, flags);
	if (cd != -1) {
		return tpgetrply(&cd, odata, olen, flags);
	} else {
		return -1;
	}
}

int AtmiBrokerConversation::tpacall(char * svc, char* idata, long ilen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpacall - svc: %s idata: %s ilen: %d flags: %d", svc, idata, ilen, flags);
	int cd = tpconnect(svc, idata, ilen, flags);
	if (cd != -1) {
		if (TPNOREPLY & flags) {
			end(cd);
			return 0;
		}
		return cd;
	} else {
		return -1;
	}
}

int AtmiBrokerConversation::tpconnect(char * serviceName, char* idata, long ilen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpconnect - svc: %s idata: %s ilen: %d flags: %d", serviceName, idata, ilen, flags);

	tperrno = 0;
	int cd = -1;

	try {
		AtmiBroker::ServiceFactory_ptr ptr = get_service_factory(serviceName);
		if (ptr) {
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "start_conversation");
			char* aId = (char*) malloc(sizeof(char*) * XATMI_SERVICE_NAME_LENGTH);
			CORBA::String_var aIor = ptr->start_conversation(aId);
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "got back ior %s", (char*) aIor);
			CORBA::Object_var tmp_ref = client_orb->string_to_object(aIor);
			AtmiBroker::Service_var aCorbaService = AtmiBroker::Service::_narrow(tmp_ref);
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "got service %p with id %s", (void*) aCorbaService, (char*) aId);

			long revent = 0;
			int response = send(aCorbaService, idata, ilen, false, flags, &revent);
			if (response != -1) {
				cd = mAtmiBrokerClient->convertIdToInt(aId);
			}
		} else {
			tperrno = TPENOENT;
		}
	} catch (...) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->start_conversation(): call failed");
		tperrno = TPESYSTEM;
	}
	return cd;
}

int AtmiBrokerConversation::tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpsend - id: %d idata: %s ilen: %d flags: %d", id, idata, ilen, flags);

	tperrno = 0;
	int toReturn = 0;

	// validate flags
	if (TPNOTRAN & flags) {
		tperrno = TPEINVAL;
		toReturn = -1;
	} else {
		char * idStr = mAtmiBrokerClient->convertIdToString(id);
		if (idStr == NULL) {
			tperrno = TPEBADDESC;
			toReturn = -1;
		} else {
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object string id is %s", idStr);
			AtmiBroker::Service_var aCorbaService;
			try {
				mAtmiBrokerClient->findService(idStr, &aCorbaService);
			} catch (...) {
				userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->findService(): call failed");
				tperrno = TPESYSTEM;
				toReturn = -1;
			}
			if (CORBA::is_nil(aCorbaService)) {
				tperrno = TPEBADDESC;
				toReturn = -1;
			} else {
				toReturn = send(aCorbaService, idata, ilen, true, flags, revent);
			}
		}
	}
	return toReturn;
}

int AtmiBrokerConversation::send(AtmiBroker::Service_var aCorbaService, char* data, long ilen, bool conversation, long flags, long *revent) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "send - data: %s ilen: %d flags: %d revent: %d", data, ilen, flags, revent);

	tperrno = 0;
	int toReturn = 0;

	char* type = NULL;
	char* subtype = NULL;
	long atype = AtmiBrokerMem::get_instance()->tptypes(data, type, subtype);
	if (atype == -1L) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "MEMORY NOT ALLOCATED THRU TPALLOC!!!");
		tperrno = TPEITYPE; // TODO THE SPEC DOES NOT SAY THIS IS NECCESARY FOR TPSEND...
		toReturn = -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "type of memory: '%s'  subtype: '%s'", type, subtype);
		try {
			if (~TPNOTRAN & flags) {
				// don't run the call in a transaction
				destroySpecific(TSS_KEY);
			}

			CORBA::Long a_ilen = ilen;
			unsigned char * data_togo = (unsigned char *) malloc(atype);
			memcpy(data_togo, data, atype);
			AtmiBroker::octetSeq * a_idata = new AtmiBroker::octetSeq(a_ilen, a_ilen, data_togo, true);
			// TODO NOTIFY SERVER OF POSSIBLE CONDITIONS
			aCorbaService->send_data(conversation, mAtmiBrokerClient->getClientCallbackIOR(), *a_idata, a_ilen, flags, 0);
		} catch (...) {
			userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->send_data(): call failed");
			tperrno = TPESYSTEM;
			toReturn = -1;
		}
	}
	return toReturn;
}

int AtmiBrokerConversation::tpgetrply(int * id, char ** odata, long *olen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpgetrply - id: %d odata: %s olen: %p flags: %d", *id, *odata, olen, flags);
	long events;
	int toReturn = tprecv(*id, odata, olen, flags, &events);
	end(*id);
	return toReturn;
}

int AtmiBrokerConversation::tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tprecv - id: %d odata: %s olen: %p flags: %d", id, *odata, olen, flags);

	tperrno = 0;
	int toReturn = 0;

	CORBA::Short a_rval;
	CORBA::Long a_rcode;
	AtmiBroker::octetSeq * a_odata;
	CORBA::Long a_olen = 0;
	CORBA::Long a_flags = flags;
	CORBA::Long a_oevent;
	CORBA::Short a_result;

	AtmiBroker::Service_var aCorbaService;

	char * idStr = mAtmiBrokerClient->convertIdToString(id);
	if (idStr == NULL) {
		tperrno = TPEBADDESC;
		toReturn = -1;
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object string id is %s", idStr);

		AtmiBroker_ClientCallbackImpl * callback = mAtmiBrokerClient->getClientCallback();
		// TODO CANNOT CHECK THE TYPE OF THE REFERENCE
		a_result = callback->dequeue_data(a_rval, a_rcode, a_odata, a_olen, a_flags, a_oevent);
		toReturn = a_result;

		if (toReturn != -1) {
			// TODO Handle TPNOCHANGE
			// populated odata and olen
			*odata = (char*) a_odata->get_buffer();
			*olen = a_olen;
			*event = a_oevent;
		} else {
			// TODO SET TPERRNO
		}
	}
	free(idStr);
	return toReturn;
}

int AtmiBrokerConversation::tpcancel(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpcancel - id: %d", id);
	CurrentImpl* currentImpl = AtmiBrokerOTS::get_instance()->getCurrentImpl();
	if (currentImpl != NULL) {
		tperrno = TPETRAN;
		return -1;
	}
	return end(id);
}

int AtmiBrokerConversation::tpdiscon(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpdiscon - id: %d", id);
	int toReturn = end(id);
	if (toReturn == 0) {
		toReturn = tx_rollback();
	}
	return toReturn;
}

int AtmiBrokerConversation::end(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "end - id: %d", id);

	tperrno = 0;
	int toReturn = 0;

	char * idStr = mAtmiBrokerClient->convertIdToString(id);
	if (idStr == NULL) {
		tperrno = TPEBADDESC;
		toReturn = -1;
	} else {
		char *serviceName = (char*) malloc(sizeof(char) * XATMI_SERVICE_NAME_LENGTH);
		char index[5];
		mAtmiBrokerClient->extractServiceAndIndex(idStr, serviceName, index);

		AtmiBroker::ServiceFactory_var aCorbaServiceFactory = get_service_factory(serviceName);
		if (CORBA::is_nil(aCorbaServiceFactory)) {
			tperrno = TPEBADDESC;
			toReturn = -1;
		} else {
			try {
				aCorbaServiceFactory->end_conversation(index);
			} catch (const CORBA::SystemException &ex) {
				userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->end_conversation(): call failed. %s", ex._name());
				tperrno = TPESYSTEM;
				toReturn = -1;
			}
		}
		free(serviceName);
	}
	free(idStr);
	return toReturn;
}
