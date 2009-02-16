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
#include "tx.h"
#include "xatmi.h"
#include "Session.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBroker.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerServer.h"
#include "ServiceWrapper.h"
#include "AtmiBrokerMem.h"
#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerXATMI(log4cxx::Logger::getLogger("loggerXATMI"));

int send(Sender* sender, const char* replyTo, char* idata, long ilen, int correlationId, long flags, long rcode, long rval) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "send - idata: %s ilen: %d flags: %d" << idata << " " << ilen << " " << flags);
	int toReturn = -1;
	try {
		void* control = getSpecific(TSS_KEY);
		if (~TPNOTRAN & flags) {
			// don't run the call in a transaction
			destroySpecific(TSS_KEY);
		}
		MESSAGE message;
		message.replyto = replyTo;
		message.data = idata;
		message.len = ilen;
		message.correlationId = correlationId;
		message.flags = flags;
		message.rcode = rcode;
		message.rval = rval;
		sender->send(message);
		setSpecific(TSS_KEY, control);
		toReturn = 0;
	} catch (...) {
		LOG4CXX_ERROR(loggerXATMI, (char*) "aCorbaService->start_conversation(): call failed");
		tperrno = TPESYSTEM;
	}
	return toReturn;
}

int receive(Session* session, char ** odata, long *olen, long flags, long* event) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "tprecv - odata: %s olen: %p flags: %d" << *odata << " " << olen << " " << flags);
	int toReturn = -1;
	MESSAGE message = session->getReceiver()->receive(flags);
	if (message.data != NULL) {
		// TODO Handle TPNOCHANGE
		// TODO Handle buffer
		// TODO USE RVAL AND RCODE
		*odata = message.data;
		*olen = message.len;
		*event = message.event;
		try {
			if (message.replyto != NULL && strcmp(message.replyto, "") != 0) {
				session->setSendTo((char*) message.replyto);
			} else {
				session->setSendTo(NULL);
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "Could not set the send to destination to: " << message.replyto);
		}
		LOG4CXX_DEBUG(loggerXATMI, (char*) "returning - %s" << *odata);
		toReturn = 0;
	} else {
		tperrno = TPETIME;
	}
	return toReturn;
}

int * _get_tperrno(void) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "_get_tperrno");
	return &_tperrno;
}

long * _get_tpurcode(void) {
	LOG4CXX_ERROR(loggerXATMI, (char*) "_get_tpurcode - Not implemented");
	return &_tpurcode;
}

int tpadvertise(char * svcname, void(*func)(TPSVCINFO *)) {
	tperrno = 0;
	int toReturn = -1;
	if (serverinit() != -1) {
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
	if (serverinit() != -1) {
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
	return AtmiBrokerMem::get_instance()->tpalloc(type, subtype, size);
}

char* tprealloc(char * addr, long size) {
	tperrno = 0;
	return AtmiBrokerMem::get_instance()->tprealloc(addr, size);
}

void tpfree(char* ptr) {
	tperrno = 0;
	AtmiBrokerMem::get_instance()->tpfree(ptr);
}

long tptypes(char* ptr, char* type, char* subtype) {
	tperrno = 0;
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
		tperrno = TPESYSTEM;
		return -1;
	}
}

int tpacall(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		int cd = -1;
		Session* session = NULL;
		try {
			session = ptrAtmiBrokerClient->createSession(cd, svc);
		} catch (...) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "tpconnect failed to connect to service queue");
			tperrno = TPENOENT;
			return -1;
		}
		if (cd != -1) {
			::send(session->getSender(), session->getReplyTo(), idata, ilen, cd, flags, 0, 0);
			if (TPNOREPLY & flags) {
				return 0;
			}
			return cd;
		} else {
			return -1;
		}
	} else {
		tperrno = TPESYSTEM;
		return -1;
	}
}

int tpconnect(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	if (clientinit() != -1) {
		int cd = -1;
		Session* session = NULL;
		try {
			session = ptrAtmiBrokerClient->createSession(cd, svc);
		} catch (...) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "tpconnect failed to connect to service queue");
			tperrno = TPENOENT;
			return -1;
		}
		if (cd != -1) {
			::send(session->getSender(), session->getReplyTo(), idata, ilen, cd, flags, 0, 0);
		}
		return cd;
	} else {
		tperrno = TPESYSTEM;
		return -1;
	}
}

int tpgetrply(int *id, char ** odata, long *olen, long flags) {
	tperrno = 0;
	int toReturn = -1;
	if (clientinit() != -1) {
		Session* session = ptrAtmiBrokerClient->getSession(*id);
		if (session == NULL) {
			tperrno = TPEBADDESC;
		} else {
			long events;
			toReturn = ::receive(session, odata, olen, flags, &events);
			ptrAtmiBrokerClient->closeSession(*id);
			LOG4CXX_DEBUG(loggerXATMI, (char*) "tpgetrply session closed");
		}
	} else {
		tperrno = TPESYSTEM;
	}
	return toReturn;
}

int tpcancel(int id) {
	tperrno = 0;
	int toReturn = -1;
	if (clientinit() != -1) {
		void* currentImpl = getSpecific(TSS_KEY);
		if (currentImpl) {
			tperrno = TPETRAN;
		}
		ptrAtmiBrokerClient->closeSession(id);
		LOG4CXX_DEBUG(loggerXATMI, (char*) "tpcancel session closed");
		toReturn = 0;
	} else {
		tperrno = TPESYSTEM;
	}
	return toReturn;
}

int tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	tperrno = 0;
	int toReturn = -1;
	Session* session = (Session*) getSpecific(SVC_SES);
	if (session != NULL && session->getId() != id) {
		session = NULL;
	}
	if (session == NULL) {
		if (clientinit() != -1) {
			session = ptrAtmiBrokerClient->getSession(id);
		} else {
			tperrno = TPESYSTEM;
		}
	}
	if (session == NULL) {
		tperrno = TPEBADDESC;
	} else {
		if (session->getSender() == NULL) {
			tperrno = TPEPROTO;
		} else {
			toReturn = ::send(session->getSender(), session->getReplyTo(), idata, ilen, id, flags, 0, 0);
		}
	}
	return toReturn;
}

int tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	tperrno = 0;
	int toReturn = -1;
	Session* session = (Session*) getSpecific(SVC_SES);
	if (session != NULL && session->getId() != id) {
		session = NULL;
	}
	if (session == NULL) {
		if (clientinit() != -1) {
			session = ptrAtmiBrokerClient->getSession(id);
		} else {
			tperrno = TPESYSTEM;
		}
	}
	if (session == NULL) {
		tperrno = TPEBADDESC;
	} else {
		toReturn = ::receive(session, odata, olen, flags, event);
	}
	return toReturn;
}

void tpreturn(int rval, long rcode, char* data, long len, long flags) {
	tperrno = 0;
	Session* session = (Session*) getSpecific(SVC_SES);
	if (session) {
		if (session->getSender() == NULL) {
			tperrno = TPEPROTO;
		} else {
			::send(session->getSender(), "", data, len, 0, flags, rval, rcode);
			::tpfree(data);
			session->setSendTo(NULL);
		}
	} else {
		tperrno = TPEPROTO;
	}
}

int tpdiscon(int id) {
	tperrno = 0;
	int toReturn = -1;
	if (clientinit() != -1) {
		LOG4CXX_DEBUG(loggerXATMI, (char*) "end - id: %d" << id);
		Session* session = ptrAtmiBrokerClient->getSession(id);
		if (session == NULL) {
			tperrno = TPEBADDESC;
		} else {
			try {
				void* currentImpl = getSpecific(TSS_KEY);
				if (currentImpl) {
					toReturn = tx_rollback();
				}
				ptrAtmiBrokerClient->closeSession(id);
				LOG4CXX_DEBUG(loggerXATMI, (char*) "tpdiscon session closed");
			} catch (...) {
				LOG4CXX_ERROR(loggerXATMI, (char*) "aCorbaService->start_conversation(): call failed");
				tperrno = TPESYSTEM;
			}
		}
	} else {
		tperrno = TPESYSTEM;
	}
	return toReturn;
}

