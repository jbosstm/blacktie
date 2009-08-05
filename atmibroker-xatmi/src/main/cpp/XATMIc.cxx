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
#include "txClient.h"
#include "xatmi.h"
#include "Session.h"
#include "AtmiBrokerClientControl.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerClient.h"
#include "AtmiBrokerServer.h"
#include "AtmiBrokerMem.h"
#include "AtmiBrokerEnv.h"

long DISCON = 0x00000003;
long timeout = -1;
bool warned = false;

// Logger for XATMIc
log4cxx::LoggerPtr loggerXATMI(log4cxx::Logger::getLogger("loggerXATMI"));

int bufferSize(char* data, int suggestedSize) {
	if (data == NULL) {
		return 0;
	}
	int data_size = ::tptypes(data, NULL, NULL);
	if (data_size >= 0) {
		if (suggestedSize <= 0 || suggestedSize > data_size) {
			return data_size;
		} else {
			return suggestedSize;
		}
	} else {
		LOG4CXX_DEBUG(loggerXATMI,
				(char*) "A NON-BUFFER WAS ATTEMPTED TO BE SENT");
		setSpecific(TPE_KEY, TSS_TPEINVAL);
		return -1;
	}

}

void setTpurcode(long rcode) {
	char* toClear = (char*) getSpecific(TPR_KEY);
	if (toClear != NULL) {
		free(toClear);
	}
	char* toStore = (char*) malloc(8 * sizeof(long));
	sprintf(toStore, "%ld", rcode);
	setSpecific(TPR_KEY, toStore);
}

int send(Session* session, const char* replyTo, char* idata, long ilen,
		int correlationId, long flags, long rval, long rcode) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "send - ilen: " << ilen << ": "
			<< "cd: " << correlationId << "flags: " << flags);
	if (flags & TPSIGRSTRT) {
		LOG4CXX_ERROR(loggerXATMI, (char*) "TPSIGRSTRT NOT SUPPORTED");
	}
	int toReturn = -1;
	if (session->getCanSend()) {
		try {
			void* control = 0;

			if (~TPNOTRAN & flags) {
				// don't run the call in a transaction
				control = disassociate_tx();
			}

			LOG4CXX_TRACE(loggerXATMI, (char*) "allocating data to go: "
					<< ilen);

			MESSAGE message;
			message.replyto = replyTo;
			message.data = idata;
			message.len = ilen;
			message.correlationId = correlationId;
			message.flags = flags;
			message.rcode = rcode;
			message.rval = rval;
			message.type = NULL;
			message.subtype = NULL;
			if (message.data != NULL) {
				message.type = (char*) malloc(MAX_TYPE_SIZE + 1);
				memset(message.type, '\0', MAX_TYPE_SIZE + 1);
				message.subtype = (char*) malloc(MAX_SUBTYPE_SIZE + 1);
				memset(message.subtype, '\0', MAX_SUBTYPE_SIZE + 1);
				::tptypes(idata, message.type, message.subtype);
			}
			if (message.type == NULL) {
				message.type = (char*) "";
			}
			if (message.subtype == NULL) {
				message.subtype = (char*) "";
			}
			if (session->send(message)) {
				toReturn = 0;
			}
			if (message.data != NULL) {
				free(message.type);
				free(message.subtype);
			}

			if (control) {
				associate_tx(control);
			}
		} catch (...) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "send: call failed");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}
	} else {
		LOG4CXX_ERROR(loggerXATMI, (char*) "Session " << correlationId
				<< "can't send");
		setSpecific(TPE_KEY, TSS_TPEPROTO);
	}

	return toReturn;
}

int receive(Session* session, char ** odata, long *olen, long flags,
		long* event) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "Receive invoked");
	int toReturn = -1;
	int len = ::bufferSize(*odata, *olen);
	if (len != -1) {
		LOG4CXX_DEBUG(loggerXATMI, (char*) "tprecv session: "
				<< session->getId() << " olen: " << olen << " flags: " << flags);
		if (flags & TPSIGRSTRT && !warned) {
			LOG4CXX_ERROR(loggerXATMI, (char*) "TPSIGRSTRT NOT SUPPORTED");
			warned = true;
		}
		if (session->getCanRecv()) {
			// TODO Make configurable Default wait time is blocking (x1000 in SynchronizableObject)
			long time = -1;
			if (TPNOBLOCK & flags) {
				time = 1;
				LOG4CXX_DEBUG(loggerXATMI, (char*) "Setting timeout to 1");
			} else if (TPNOTIME & flags) {
				time = 0;
				LOG4CXX_DEBUG(loggerXATMI, (char*) "TPNOTIME = BLOCKING CALL");
			} else {
				if (timeout == -1) {
					timeout = (long) (atoi(
							AtmiBrokerEnv::get_instance()->getenv(
									(char*) "RequestTimeout")));
				}
				time = timeout;
			}
			LOG4CXX_DEBUG(loggerXATMI, (char*) "Setting timeout to: " << time);
			try {
				MESSAGE message = session->receive(time);
				if (message.received) {
					if (message.rval != DISCON) {
						if (flags & TPNOCHANGE) {
							char* type = message.type;
							char* subtype = message.subtype;
							char* messageType = (char*) malloc(MAX_TYPE_SIZE);
							char* messageSubtype = (char*) malloc(
									MAX_SUBTYPE_SIZE);
							tptypes(*odata, messageType, messageSubtype);
							bool changed = strncmp(type, messageType, MAX_TYPE_SIZE) != 0
									|| strncmp(subtype, messageSubtype, MAX_SUBTYPE_SIZE) != 0;
							free(messageType);
							free(messageSubtype);
							if (changed) {
								setSpecific(TPE_KEY, TSS_TPEOTYPE);
								free(message.data);
								free(message.type);
								free(message.subtype);
								free((char*) message.replyto);
								return toReturn;
							}
						}
						if (len < message.len) {
							*odata = ::tprealloc(*odata, message.len);
						}
						*olen = message.len;
						if (message.len > 0) {
							memcpy(*odata, (char*) message.data, *olen);
						} else if (message.data == NULL) {
							*odata = NULL;
						}
						free(message.data);
						free(message.type);
						free(message.subtype);
						free((char*) message.replyto);
					} else {
						*event = TPEV_DISCONIMM;
					}

					if (message.rcode == TPESVCFAIL) {
						setTpurcode(message.rcode);
						*event = TPEV_SVCFAIL;
						setSpecific(TPE_KEY, TSS_TPESVCFAIL);
					} else if (message.rcode == TPESVCERR) {
						*event = TPEV_SVCERR;
						setSpecific(TPE_KEY, TSS_TPESVCERR);
					} else if (message.rval == TPSUCCESS) {
						toReturn = 0;
						setTpurcode(message.rcode);
						*event = TPEV_SVCSUCC;
					} else if (message.flags & TPRECVONLY) {
						toReturn = 0;
						*event = TPEV_SENDONLY;
						session->setCanSend(true);
						session->setCanRecv(false);
						LOG4CXX_DEBUG(
								loggerXATMI,
								(char*) "receive TPRECVONLY set constraints session: "
										<< session->getId() << " send: "
										<< session->getCanSend() << " recv: "
										<< session->getCanRecv());
					} else if (message.flags & TPSENDONLY) {
						toReturn = 0;
						session->setCanSend(true);
						session->setCanRecv(false);
						LOG4CXX_DEBUG(
								loggerXATMI,
								(char*) "receive TPSENDONLY set constraints session: "
										<< session->getId() << " send: "
										<< session->getCanSend() << " recv: "
										<< session->getCanRecv());
					} else if (message.correlationId >= 0) {
						toReturn = 0;
					} else {
						setSpecific(TPE_KEY, TSS_TPETIME);
					}
				}
			} catch (...) {
				LOG4CXX_ERROR(
						loggerXATMI,
						(char*) "Could not set the receive from the destination");
			}
		} else {
			LOG4CXX_DEBUG(loggerXATMI, (char*) "Session can't receive");
			setSpecific(TPE_KEY, TSS_TPEPROTO);
		}
	} else {
		setSpecific(TPE_KEY, TSS_TPEOTYPE);
	}
	return toReturn;
}

int _get_tperrno(void) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "_get_tperrno");
	char* err = (char*) getSpecific(TPE_KEY);
	int toReturn = 0;
	if (err != NULL) {
		LOG4CXX_TRACE(loggerXATMI, (char*) "found _get_tperrno" << err);
		toReturn = atoi(err);
	}
	LOG4CXX_DEBUG(loggerXATMI, (char*) "returning _get_tperrno" << toReturn);
	return toReturn;
}

long _get_tpurcode(void) {
	LOG4CXX_DEBUG(loggerXATMI, (char*) "_get_tpurcode");
	char* rcode = (char*) getSpecific(TPR_KEY);
	long toReturn = 0;
	if (rcode != NULL) {
		LOG4CXX_TRACE(loggerXATMI, (char*) "found _get_tpurcode" << rcode);
		toReturn = atol(rcode);
	}
	LOG4CXX_DEBUG(loggerXATMI, (char*) "returning _get_tpurcode" << toReturn);
	return toReturn;
}

int tpadvertise(char * svcname, void(*func)(TPSVCINFO *)) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpadvertise: " << svcname);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (ptrServer != NULL) {
		if (ptrServer->advertiseService(svcname, func)) {
			toReturn = 0;
		}
	} else {
		LOG4CXX_ERROR(loggerXATMI, (char*) "server not initialized");
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpadvertise return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpunadvertise(char * svcname) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpunadvertise: " << svcname);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (ptrServer != NULL) {
		if (svcname && strcmp(svcname, "") != 0) {
			if (ptrServer->isAdvertised(svcname)) {
				ptrServer->unadvertiseService(svcname);
				toReturn = 0;
			} else {
				setSpecific(TPE_KEY, TSS_TPENOENT);
			}
		} else {
			setSpecific(TPE_KEY, TSS_TPEINVAL);
		}
	} else {
		LOG4CXX_ERROR(loggerXATMI, (char*) "server not initialized");
		setSpecific(TPE_KEY, TSS_TPESYSTEM);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpunadvertise return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

char* tpalloc(char* type, char* subtype, long size) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpalloc type: " << type << " size: "
			<< size);
	if (subtype) {
		LOG4CXX_TRACE(loggerXATMI, (char*) "tpalloc subtype: " << type);
	}
	setSpecific(TPE_KEY, TSS_TPERESET);
	char* toReturn = NULL;
	if (clientinit() != -1) {
		toReturn = AtmiBrokerMem::get_instance()->tpalloc(type, subtype, size);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpalloc returning" << " tperrno: "
			<< tperrno);
	return toReturn;
}

char* tprealloc(char * addr, long size) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tprealloc: " << size);
	setSpecific(TPE_KEY, TSS_TPERESET);
	char* toReturn = NULL;
	if (clientinit() != -1) {
		toReturn = AtmiBrokerMem::get_instance()->tprealloc(addr, size);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tprealloc returning" << " tperrno: "
			<< tperrno);
	return toReturn;
}

void tpfree(char* ptr) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpfree");
	setSpecific(TPE_KEY, TSS_TPERESET);
	if (clientinit() != -1) {
		AtmiBrokerMem::get_instance()->tpfree(ptr);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpfree returning" << " tperrno: "
			<< tperrno);
}

long tptypes(char* ptr, char* type, char* subtype) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tptypes called");
	setSpecific(TPE_KEY, TSS_TPERESET);
	LOG4CXX_TRACE(loggerXATMI, (char*) "set the specific");
	long toReturn = -1;
	if (clientinit() != -1) {
		toReturn = AtmiBrokerMem::get_instance()->tptypes(ptr, type, subtype);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tptypes return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen,
		long flags) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpcall: " << svc << " ilen: " << ilen
			<< " flags: " << flags);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
		int cd = tpacall(svc, idata, ilen, flags);
		if (cd != -1) {
			toReturn = tpgetrply(&cd, odata, olen, flags);
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpcall return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpacall(char * svc, char* idata, long ilen, long flags) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpacall: " << svc << " ilen: " << ilen
			<< " flags: " << flags);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int len = ::bufferSize(idata, ilen);
	int toReturn = -1;
	if (len != -1) {
		if (clientinit() != -1) {
			Session* session = NULL;
			int cd = -1;
			try {
				session = ptrAtmiBrokerClient->createSession(cd, svc);
				if (cd != -1) {
					toReturn = ::send(session, session->getReplyTo(), idata,
							len, cd, flags, 0, 0);
					if (toReturn >= 0) {
						if (TPNOREPLY & flags) {
							toReturn = 0;
							ptrAtmiBrokerClient->closeSession(cd);
						} else {
							toReturn = cd;
						}
					} else {
						LOG4CXX_DEBUG(loggerXATMI,
								(char*) "Session got dudded: " << cd);
						ptrAtmiBrokerClient->closeSession(cd);
					}
				} else {
					setSpecific(TPE_KEY, TSS_TPELIMIT);
					ptrAtmiBrokerClient->closeSession(cd);
				}
			} catch (...) {
				LOG4CXX_ERROR(loggerXATMI,
						(char*) "tpacall failed to connect to service queue");
				setSpecific(TPE_KEY, TSS_TPENOENT);
				if (cd != -1) {
					ptrAtmiBrokerClient->closeSession(cd);
				}
			}
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpacall return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpconnect(char * svc, char* idata, long ilen, long flags) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpconnect: " << svc << " ilen: "
			<< ilen << " flags: " << flags);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
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
						int sendOk = ::send(session, session->getReplyTo(),
								idata, len, cd, flags | TPCONV, 0, 0);
						if (sendOk != -1) {
							long olen = 4;
							char* odata = (char*) tpalloc((char*) "X_OCTET",
									NULL, olen);
							long event = 0;
							::tprecv(cd, &odata, &olen, flags, &event);
							bool connected = strcmp(odata, "ACK") == 0;
							tpfree(odata);
							if (connected) {
								toReturn = cd;
								if (flags & TPRECVONLY) {
									session->setCanSend(false);
									LOG4CXX_DEBUG(
											loggerXATMI,
											(char*) "tpconnect set constraints session: "
													<< session->getId()
													<< " send: "
													<< session->getCanSend()
													<< " recv (not changed): "
													<< session->getCanRecv());
								} else {
									session->setCanRecv(false);
									LOG4CXX_DEBUG(
											loggerXATMI,
											(char*) "tpconnect set constraints session: "
													<< session->getId()
													<< " send (not changed): "
													<< session->getCanSend()
													<< " recv: "
													<< session->getCanRecv());
								}
							} else {
								LOG4CXX_DEBUG(loggerXATMI,
										(char*) "COULD NOT CONNECT: " << cd);
								ptrAtmiBrokerClient->closeSession(cd);
								setSpecific(TPE_KEY, TSS_TPESYSTEM);
							}
						} else {
							LOG4CXX_DEBUG(loggerXATMI,
									(char*) "Session got dudded: " << cd);
							ptrAtmiBrokerClient->closeSession(cd);
						}
					} else {
						setSpecific(TPE_KEY, TSS_TPELIMIT);
					}
				} catch (...) {
					LOG4CXX_ERROR(
							loggerXATMI,
							(char*) "tpconnect failed to connect to service queue");
					setSpecific(TPE_KEY, TSS_TPENOENT);
					if (cd != -1) {
						ptrAtmiBrokerClient->closeSession(cd);
					}
				}
			}
		}
	} else {
		setSpecific(TPE_KEY, TSS_TPEINVAL);
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpconnect return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpgetrply(int *id, char ** odata, long *olen, long flags) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpgetrply " << id);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
		if (id && olen) {
			Session* session = ptrAtmiBrokerClient->getSession(*id);
			if (session == NULL) {
				setSpecific(TPE_KEY, TSS_TPEBADDESC);
			} else {
				long event = 0;
				toReturn = ::receive(session, odata, olen, flags, &event);
				ptrAtmiBrokerClient->closeSession(*id);
				if (event == TPESVCERR) {
					setSpecific(TPE_KEY, TSS_TPESVCERR);
					toReturn = -1;
				} else if (event == TPESVCFAIL) {
					setSpecific(TPE_KEY, TSS_TPESVCFAIL);
					toReturn = -1;
				}
				LOG4CXX_TRACE(loggerXATMI, (char*) "tpgetrply session closed");
			}
		} else {
			setSpecific(TPE_KEY, TSS_TPEINVAL);
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpgetrply return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpcancel(int id) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpcancel " << id);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
		if (getSpecific(TSS_KEY)) {
			setSpecific(TPE_KEY, TSS_TPETRAN);
		}
		if (ptrAtmiBrokerClient->getSession(id) != NULL) {
			ptrAtmiBrokerClient->closeSession(id);
			LOG4CXX_TRACE(loggerXATMI, (char*) "tpcancel session closed");
			toReturn = 0;
		} else {
			setSpecific(TPE_KEY, TSS_TPEBADDESC);
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpcancel return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpsend " << id);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
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
					setSpecific(TPE_KEY, TSS_TPEBADDESC);
				} else {
					if (!session->getCanSend()) {
						LOG4CXX_DEBUG(loggerXATMI, (char*) "Session can't send");
						setSpecific(TPE_KEY, TSS_TPEPROTO);
					} else {
						len = ::bufferSize(idata, ilen);
					}
				}
			}
		}
		if (len != -1) {
			toReturn = ::send(session, session->getReplyTo(), idata, len, id,
					flags | TPCONV, 0, 0);
			if (toReturn != -1 && flags & TPRECVONLY) {
				session->setCanSend(false);
				session->setCanRecv(true);
				LOG4CXX_DEBUG(loggerXATMI,
						(char*) "tpsend set constraints session: "
								<< session->getId() << " send: "
								<< session->getCanSend() << " recv: "
								<< session->getCanRecv());
			}
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpsend return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

int tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tprecv " << id);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
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
			setSpecific(TPE_KEY, TSS_TPEBADDESC);
		} else {
			toReturn = ::receive(session, odata, olen, flags, event);
			if (*event == TPEV_SVCERR) {
				setSpecific(TPE_KEY, TSS_TPESVCERR);
				ptrAtmiBrokerClient->closeSession(id);
				toReturn = -1;
			} else if (*event == TPEV_SVCFAIL) {
				setSpecific(TPE_KEY, TSS_TPESVCFAIL);
				ptrAtmiBrokerClient->closeSession(id);
				toReturn = -1;
			} else if (*event == TPEV_SVCSUCC) {
				setSpecific(TPE_KEY, TSS_TPEEVENT);
				ptrAtmiBrokerClient->closeSession(id);
				toReturn = -1;
			}
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tprecv return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}

void tpreturn(int rval, long rcode, char* data, long len, long flags) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpreturn " << rval);
	setSpecific(TPE_KEY, TSS_TPERESET);
	if (clientinit() != -1) {
		Session* session = (Session*) getSpecific(SVC_SES);
		if (session != NULL) {
			if (!session->getCanSend() && rval != TPFAIL && data != NULL) {
				rcode = TPESVCERR;
			}
			session->setCanRecv(false);
			// CANT SEND NULL DATA
			if (data == NULL) {
				data = ::tpalloc((char*) "X_OCTET", NULL, 0);
			}
			if (rcode == TPESVCERR || bufferSize(data, len) == -1) {
				::tpfree(data);
				data = ::tpalloc((char*) "X_OCTET", NULL, 0);
				::send(session, "", data, 0, 0, flags, TPFAIL, TPESVCERR);
			} else {
				if (rval != TPSUCCESS && rval != TPFAIL) {
					rval = TPFAIL;
					//TODO MARK SET ROLLBACK ONLY
				}
				::send(session, "", data, len, 0, flags, rval, rcode);
			}
			::tpfree(data);
			session->setSendTo(NULL);
			session->setCanSend(false);
			LOG4CXX_DEBUG(loggerXATMI,
					(char*) "tpreturn set constraints session: "
							<< session->getId() << " send: "
							<< session->getCanSend() << " recv: "
							<< session->getCanRecv());

		} else {
			LOG4CXX_DEBUG(loggerXATMI, (char*) "Session is null");
			setSpecific(TPE_KEY, TSS_TPEPROTO);
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpreturn returning" << " tperrno: "
			<< tperrno);
}

int tpdiscon(int id) {
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpdiscon " << id);
	setSpecific(TPE_KEY, TSS_TPERESET);
	int toReturn = -1;
	if (clientinit() != -1) {
		LOG4CXX_DEBUG(loggerXATMI, (char*) "end - id: " << id);
		Session* session = ptrAtmiBrokerClient->getSession(id);
		if (session == NULL) {
			setSpecific(TPE_KEY, TSS_TPEBADDESC);
		} else {
			// SEND THE DISCONNECT TO THE REMOTE SIDE
			long flags = 0;
			char* data = ::tpalloc((char*) "X_OCTET", NULL, 0);
			//data[0] = NULL; // TODO SHOULD BE ABLE TO SEND ZERO LENGTH BUFFERS
			::send(session, "", data, 0, id, flags, DISCON, 0);
			try {
				if (getSpecific(TSS_KEY)) {
					toReturn = tx_rollback();
				}
				ptrAtmiBrokerClient->closeSession(id);
				LOG4CXX_TRACE(loggerXATMI, (char*) "tpdiscon session closed");
			} catch (...) {
				LOG4CXX_ERROR(loggerXATMI, (char*) "tpdiscon: call failed");
				setSpecific(TPE_KEY, TSS_TPESYSTEM);
			}
		}
	}
	LOG4CXX_TRACE(loggerXATMI, (char*) "tpdiscon return: " << toReturn
			<< " tperrno: " << tperrno);
	return toReturn;
}
