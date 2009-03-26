/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "log4cxx/logger.h"

#include "ThreadLocalStorage.h"
#include "tx.h"
#include "xatmi.h"
#include "Session.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerMem.h"

// Global state
int _tperrno = 0;
long _tpurcode = -1;

// Logger for XATMIc
log4cxx::LoggerPtr loggerXATMI(log4cxx::Logger::getLogger("loggerXATMI"));

int bufferSize(char* data, int suggestedSize) {
	int data_size = ::tptypes(data, NULL, NULL);
	if (data_size >= 0) {
		if (suggestedSize <= 0 || suggestedSize > data_size) {
			return data_size;
		} else {
			return suggestedSize;
		}
	} else {
		LOG4CXX_DEBUG(loggerXATMI, (char*) "A NON-BUFFER WAS ATTEMPTED TO BE SENT");
		tperrno = TPEINVAL;
		return -1;
	}

}
int send(Session* session, const char* replyTo, char* idata, long ilen, int correlationId, long flags, long rval, long rcode) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "send - idata: %s ilen: %d flags: %d" << idata << " " << ilen << " " << flags);
	if (flags & TPSIGRSTRT) {
		LOG4CXX_ERROR(loggerXATMI, (char*) "TPSIGRSTRT NOT SUPPORTED");
	}
	int toReturn = -1;
	if (session->getCanSend()) {
		try {
			bool transactionSuspended = false;
			void* control = getSpecific(TSS_KEY);
			if (control && (~TPNOTRAN & flags)) {
				// don't run the call in a transaction
				destroySpecific(TSS_KEY);
				transactionSuspended = true;
			}

			MESSAGE message;
			message.replyto = replyTo;
			message.data = idata;
			message.len = ilen;
			message.correlationId = correlationId;
			message.flags = flags;
			message.rcode = rcode;
			message.rval = rval;
			session->send(message);
			toReturn = 0;

			if (transactionSuspended) {
				setSpecific(TSS_KEY, control);
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "aCorbaService->start_conversation(): call failed");
			tperrno = TPESYSTEM;
		}
	} else {
		tperrno = TPEPROTO;
	}

	return toReturn;
}

int receive(Session* session, char ** odata, long *olen, long flags, long* event) {
	int toReturn = -1;
	int len = ::bufferSize(*odata, *olen);
	if (len != -1) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "tprecv - odata: %s olen: %p flags: %d" << *odata << " " << olen << " " << flags);
	if (flags & TPSIGRSTRT) {
		LOG4CXX_ERROR(loggerXATMI, (char*) "TPSIGRSTRT NOT SUPPORTED");
	}
	if (session->getCanRecv()) {
		// TODO Make configurable Default wait time is blocking (x1000 in SynchronizableObject)
		long time = 0;
		if (TPNOBLOCK & flags) {
			time = 1;
		} else if (TPNOTIME & flags) {
			time = 0;
		}
		MESSAGE message = session->receive(time);
		if (message.data != NULL) {
			// TODO Handle TPNOCHANGE
			// TODO Handle buffer
			// TODO USE RVAL AND RCODE AND EVENT
			if (len < message.len) {
				*odata = ::tprealloc(*odata, message.len);
			}
			*olen = message.len;
			memcpy(*odata, (char*) message.data, *olen);
			if (message.rcode == TPESVCFAIL) {
				*event = TPESVCFAIL;
			} else if (message.rcode == TPESVCERR) {
				*event = TPESVCERR;
			}
			try {
				if (message.replyto != NULL && strcmp(message.replyto, "") != 0) {
					session->setSendTo(message.replyto);
				} else {
					session->setSendTo(NULL);
				}
				if (message.flags & TPRECVONLY) {
					session->setCanSend(true);
					session->setCanRecv(false);
				}
			} catch (...) {
				LOG4CXX_ERROR(loggerXATMI, (char*) "Could not set the send to destination to: " << message.replyto);
			}
			LOG4CXX_DEBUG(loggerXATMI, (char*) "returning - %s" << *odata);
			toReturn = 0;
		} else {
			tperrno = TPETIME;
		}
	} else {
		tperrno = TPEPROTO;
	}
	} else {
		tperrno = TPEOTYPE;
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
		return -1;
	}
}

int tpacall(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	int len = ::bufferSize(idata, ilen);
	if (len != -1) {
		if (clientinit() != -1) {
			Session* session = NULL;
			try {
				int cd = -1;
				session = ptrAtmiBrokerClient->createSession(cd, svc);
				if (cd != -1) {
					::send(session, session->getReplyTo(), idata, len, cd, flags, 0, 0);
					if (TPNOREPLY & flags) {
						return 0;
					}
					return cd;
				} else {
					tperrno = TPELIMIT;
				}
			} catch (...) {
				LOG4CXX_ERROR(loggerXATMI, (char*) "tpconnect failed to connect to service queue");
				tperrno = TPENOENT;
			}
		}
	}
	return -1;
}

int tpconnect(char * svc, char* idata, long ilen, long flags) {
	tperrno = 0;
	if (flags & TPSENDONLY || flags & TPRECVONLY) {
		int len = 0;
		if (idata != NULL) {
			len = ::bufferSize(idata, ilen);
		}
		if (len != -1) {
			if (clientinit() != -1) {
				int cd = -1;
				Session* session = NULL;
				try {
					session = ptrAtmiBrokerClient->createSession(cd, svc);
					if (cd != -1) {
						::send(session, session->getReplyTo(), idata, len, cd, flags, 0, 0);
						if (flags & TPRECVONLY) {
							session->setCanSend(false);
						} else {
							session->setCanRecv(false);
						}
						return cd;
					} else {
						tperrno = TPELIMIT;
					}
				} catch (...) {
					LOG4CXX_ERROR(loggerXATMI, (char*) "tpconnect failed to connect to service queue");
					tperrno = TPENOENT;
				}
			}
		}
	} else {
		tperrno = TPEINVAL;
	}
	return -1;
}

int tpgetrply(int *id, char ** odata, long *olen, long flags) {
	tperrno = 0;
	int toReturn = -1;
	if (clientinit() != -1) {
		if (id && olen) {
			Session* session = ptrAtmiBrokerClient->getSession(*id);
			if (session == NULL) {
				tperrno = TPEBADDESC;
			} else {
				long event;
				toReturn = ::receive(session, odata, olen, flags, &event);
				ptrAtmiBrokerClient->closeSession(*id);
				if (event == TPESVCERR) {
					tperrno = TPESVCERR;
					toReturn = -1;
				} else if (event == TPESVCFAIL) {
					tperrno = TPESVCFAIL;
					toReturn = -1;
				}
				LOG4CXX_DEBUG(loggerXATMI, (char*) "tpgetrply session closed");
			}
		} else {
			tperrno = TPEINVAL;
		}
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
		if (ptrAtmiBrokerClient->getSession(id) != NULL) {
			ptrAtmiBrokerClient->closeSession(id);
			LOG4CXX_DEBUG(loggerXATMI, (char*) "tpcancel session closed");
			toReturn = 0;
		} else {
			tperrno = TPEBADDESC;
		}
	}
	return toReturn;
}

int tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	tperrno = 0;
	int toReturn = -1;
	Session* session = (Session*) getSpecific(SVC_SES);
	int len = -1;
	if (session != NULL) {
		if (session->getId() != id) {
			session = NULL;
		} else {
			len = ilen;
		}
	}
	if (session == NULL) {
		if (clientinit() != -1) {
			session = ptrAtmiBrokerClient->getSession(id);
			if (session == NULL) {
				tperrno = TPEBADDESC;
			} else {
				if (!session->getCanSend()) {
					tperrno = TPEPROTO;
				} else {
					len = ::bufferSize(idata, ilen);
				}
			}
		}
	}
	if (len != -1) {
		toReturn = ::send(session, session->getReplyTo(), idata, len, id, flags, 0, 0);
		if (flags & TPRECVONLY) {
			session->setCanSend(false);
			session->setCanRecv(true);
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
		}
	}
	if (session == NULL) {
		tperrno = TPEBADDESC;
	} else {
		toReturn = ::receive(session, odata, olen, flags, event);
		if (*event == TPESVCERR) {
			tperrno = TPEEVENT;
			toReturn = -1;
		} else if (*event == TPESVCFAIL) {
			tperrno = TPEEVENT;
			toReturn = -1;
		}
	}
	return toReturn;
}

void tpreturn(int rval, long rcode, char* data, long len, long flags) {
	tperrno = 0;
	Session* session = (Session*) getSpecific(SVC_SES);
	if (session != NULL) {
		if (!session->getCanSend()) {
			tperrno = TPEPROTO;
		} else {
			session->setCanRecv(false);
			if (bufferSize(data, len) == -1) {
				::send(session, "", data, 0, 0, flags, TPFAIL, TPESVCERR);
			} else {
				::send(session, "", data, len, 0, flags, rval, rcode);
			}
			::tpfree(data);
			session->setSendTo(NULL);
			session->setCanSend(false);
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
	}
	return toReturn;
}

