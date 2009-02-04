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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "ThreadLocalStorage.h"
#include "userlog.h"
#include "tx.h"
#include "xatmi.h"
#include "Session.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBroker.h"
#include "AtmiBrokerConversation.h"
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBrokerMem.h"
#include "log4cxx/logger.h"



log4cxx::LoggerPtr loggerXATMI(log4cxx::Logger::getLogger("loggerXATMI"));

int _tperrno = 0;
long _tpurcode = -1;

int * _get_tperrno(void) {
	userlog(log4cxx::Level::getDebug(), loggerXATMI, (char*) "_get_tperrno");
	return &_tperrno;
}

long * _get_tpurcode(void) {
	userlog(log4cxx::Level::getError(), loggerXATMI, (char*) "_get_tpurcode - Not implemented");
	return &_tpurcode;
}

int tpadvertise(char * svcname, void(*func)(TPSVCINFO *)) {
	tperrno = 0;
	int toReturn = -1;
	if (ptrServer) {
		if (ptrServer->advertiseService(svcname, func)) {
			toReturn = 0;
		}
	} else {
		tperrno = TPEPROTO;
	}
	return toReturn;
}

int tpunadvertise(char * svcname) {
	tperrno = 0;
	int toReturn = -1;
	if (ptrServer) {
		if (svcname && strcmp(svcname, "") != 0) {
			if (ptrServer->isAdvertised(svcname)) {
				ptrServer->unadvertiseService(svcname);
				toReturn = 0;
			} else {
				tperrno = TPENOENT;
			}
		} else {
			tperrno = TPEINVAL;
		}
	} else {
		tperrno = TPEPROTO;
	}
	return toReturn;
}

char* tpalloc(char* type, char* subtype, long size) {
	tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerXATMI, (char*) "tpalloc - type: '%s' size: %d", type, size);
	return AtmiBrokerMem::get_instance()->tpalloc(type, subtype, size);
}

char* tprealloc(char * addr, long size) {
	tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerXATMI, (char*) "tprealloc - addr: %p size: %d", addr, size);
	return AtmiBrokerMem::get_instance()->tprealloc(addr, size);
}

void tpfree(char* ptr) {
	tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerXATMI, (char*) "tpfree - ptr: %p", ptr);
	AtmiBrokerMem::get_instance()->tpfree(ptr);
}

long tptypes(char* ptr, char* type, char* subtype) {
	tperrno = 0;
	userlog(log4cxx::Level::getDebug(), loggerXATMI, (char*) "tptypes - ptr: %p %s", ptr, type);
	return AtmiBrokerMem::get_instance()->tptypes(ptr, type, subtype);
}

int tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		int cd = tpacall(svc, idata, ilen, flags);
		if (cd != -1) {
			return tpgetrply(&cd, odata, olen, flags);
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int tpacall(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		int cd = tpconnect(svc, idata, ilen, flags);
		if (cd != -1) {
			if (TPNOREPLY & flags) {
				return 0;
			}
			return cd;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

int tpconnect(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		Sender* ptr = NULL;
		try {
			ptr = ::get_service_queue(svc);
		} catch (...) {
			userlog(log4cxx::Level::getError(), loggerXATMI, (char*) "tpconnect get_service_queue failed");
			tperrno = TPENOENT;
			return -1;
		}
		Session* session = ptrAtmiBrokerClient->createSession();
		long revent = 0;
		AtmiBrokerConversation::get_instance()->send(ptr, session->getReceiver()->getDestinationName(), idata, ilen, flags, &revent);
		int id = -1;
		session->getId(id);
		return id;
	} else {
		return -1;
	}
}

int tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	tperrno = 0;
	Session* session = (Session*) getSpecific(SVC_KEY);
	if (session == NULL) {
		if (clientinit() != -1) {
			session = ptrAtmiBrokerClient->getSession(&id);
		} else {
			tperrno = TPESYSTEM;
		}
	}
	if (session == NULL) {
		tperrno = TPEBADDESC;
		return -1;
	}
	return AtmiBrokerConversation::get_instance()->send(session->getSender(), session->getReceiver()->getDestinationName(), idata, ilen, flags, revent);
}

int tpgetrply(int *id, char ** odata, long *olen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		Session* session = ptrAtmiBrokerClient->getSession(id);
		long events;
		int toReturn = AtmiBrokerConversation::get_instance()->receive(session, odata, olen, flags, &events);
		return toReturn;
	} else {
		return -1;
	}
}

int tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	tperrno = 0;
	Session* session = (Session*) getSpecific(SVC_KEY);
	if (session == NULL) {
		if (clientinit() != -1) {
			session = ptrAtmiBrokerClient->getSession(&id);
		} else {
			tperrno = TPESYSTEM;
		}
	}
	if (session == NULL) {
		tperrno = TPEBADDESC;
	}
	return AtmiBrokerConversation::get_instance()->receive(session, odata, olen, flags, event);
}

void tpreturn(int rval, long rcode, char* data, long len, long flags) {
	tperrno = 0;
	Session* session = (Session*) getSpecific(SVC_KEY);
	if (session) {
		long event = 0;
		AtmiBrokerConversation::get_instance()->send(session->getSender(), "", data, len, flags, &event);
	} else {
		tperrno = TPEPROTO;
	}
}

int tpdiscon(int id) {
	tperrno = 0;
	if (clientinit() != -1) {
		int toReturn = AtmiBrokerConversation::get_instance()->disconnect(id);
		if (toReturn == 0) {
			void* currentImpl = getSpecific(TSS_KEY);
			if (currentImpl) {
				return tx_rollback();
			}
		}
		return toReturn;
	} else {
		return -1;
	}
}

int tpcancel(int id) {
	tperrno = 0;
	if (clientinit() != -1) {
		void* currentImpl = getSpecific(TSS_KEY);
		if (currentImpl) {
			tperrno = TPETRAN;
			return -1;
		}
		return AtmiBrokerConversation::get_instance()->disconnect(id);
	} else {
		return -1;
	}
}
