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
#endif

#include "atmiBrokerMacro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "ThreadLocalStorage.h"
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
			disconnect(cd);
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
		AtmiBroker::ServiceQueue_ptr ptr = mAtmiBrokerClient->get_service_queue(serviceName);
		if (ptr) {
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "start_conversation");

			if (~TPNOTRAN & flags) {
				// don't run the call in a transaction
				destroySpecific(TSS_KEY);
			}

			long dataLength = ::tptypes(idata, NULL, NULL);
			if (dataLength >= 0) {
				if (ilen > 0 && ilen < dataLength) {
					dataLength = ilen;
				}
				int id = 1;
				unsigned char * data_togo = (unsigned char *) malloc(sizeof(char*) * dataLength);
				memcpy(data_togo, idata, dataLength);
				AtmiBroker::octetSeq * a_idata = new AtmiBroker::octetSeq(dataLength, dataLength, data_togo, true);
				// TODO NOTIFY SERVER OF POSSIBLE CONDITIONS
				ptr->send(mAtmiBrokerClient->getLocalCallback(id)->getReplyTo(), *a_idata, dataLength, flags);
				cd = id;
			} else {
				tperrno = TPEINVAL;
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

	//	// validate flags
	//	if (TPNOTRAN & flags) {
	//		tperrno = TPEINVAL;
	//		toReturn = -1;
	//	} else {
	//		char * idStr = mAtmiBrokerClient->convertIdToString(id);
	//		if (idStr == NULL) {
	//			tperrno = TPEBADDESC;
	//			toReturn = -1;
	//		} else {
	//			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "object string id is %s", idStr);
	//			AtmiBroker::Service_var aCorbaService;
	//			try {
	//				mAtmiBrokerClient->getRemoteCallback(idStr, &aCorbaService);
	//			} catch (...) {
	//				userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->findService(): call failed");
	//				tperrno = TPESYSTEM;
	//				toReturn = -1;
	//			}
	//			if (CORBA::is_nil(aCorbaService)) {
	//				tperrno = TPEBADDESC;
	//				toReturn = -1;
	//			} else {
	//				toReturn = send(aCorbaService, idata, ilen, true, flags, revent);
	//			}
	//		}
	//	}
	return toReturn;
}

int AtmiBrokerConversation::tpgetrply(int * id, char ** odata, long *olen, long flags) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpgetrply - id: %d odata: %s olen: %p flags: %d", *id, *odata, olen, flags);
	long events;
	int toReturn = tprecv(*id, odata, olen, flags, &events);
	return toReturn;
}

int AtmiBrokerConversation::tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tprecv - id: %d odata: %s olen: %p flags: %d", id, *odata, olen, flags);

	tperrno = 0;
	int toReturn = 0;

	EndpointQueue * callback = mAtmiBrokerClient->getLocalCallback(id);
	if (callback == NULL) {
		tperrno = TPEBADDESC;
		toReturn = -1;
	} else {
		MESSAGE message = callback->receive(flags);
		if (message.idata != NULL) {
			// TODO Handle TPNOCHANGE
			// populated odata and olen
			// TODO USE RVAL AND RCODE
			*odata = message.idata;
			*olen = message.ilen;
			*event = message.event;
			userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "returning - %s", *odata);
		} else {
			tperrno = TPETIME;
			toReturn = -1;
		}
	}
	return toReturn;
}

int AtmiBrokerConversation::tpcancel(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpcancel - id: %d", id);
	CurrentImpl* currentImpl = AtmiBrokerOTS::get_instance()->getCurrentImpl();
	if (currentImpl != NULL) {
		tperrno = TPETRAN;
		return -1;
	}
	return disconnect(id);
}

int AtmiBrokerConversation::tpdiscon(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpdiscon - id: %d", id);
	int toReturn = disconnect(id);
	if (toReturn == 0) {
		toReturn = tx_rollback();
	}
	return toReturn;
}

int AtmiBrokerConversation::disconnect(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "end - id: %d", id);

	tperrno = 0;
	int toReturn = -1;

	EndpointQueue * callback = mAtmiBrokerClient->getRemoteCallback(id);
	if (callback == NULL) {
		tperrno = TPEBADDESC;
	} else {
		try {
			callback->disconnect();
			toReturn = 0;
		} catch (...) {
			userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "callback->disconnect(): call failed");
			tperrno = TPESYSTEM;
		}
	}
	return toReturn;
}
