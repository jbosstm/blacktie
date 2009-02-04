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

#include "ThreadLocalStorage.h"
#include "userlog.h"
#include "xatmi.h"
#include "AtmiBrokerConversation.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerConversation(Logger::getLogger("AtmiBrokerConversation"));

AtmiBrokerConversation *AtmiBrokerConversation::ptrAtmiBrokerConversation = NULL;

AtmiBrokerConversation *
AtmiBrokerConversation::get_instance() {
	if (ptrAtmiBrokerConversation == NULL)
		ptrAtmiBrokerConversation = new AtmiBrokerConversation();
	return ptrAtmiBrokerConversation;
}

void AtmiBrokerConversation::discard_instance() {
	if (ptrAtmiBrokerConversation != NULL) {
		delete ptrAtmiBrokerConversation;
		ptrAtmiBrokerConversation = NULL;
	}
}

AtmiBrokerConversation::AtmiBrokerConversation() {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "constructor");
}

AtmiBrokerConversation::~AtmiBrokerConversation() {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "destructor");
}

int AtmiBrokerConversation::send(Sender* sender, const char* replyTo, char* idata, long ilen, long flags, long *revent) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tpconnect - idata: %s ilen: %d flags: %d", idata, ilen, flags);
	int toReturn = 0;
	try {
		if (~TPNOTRAN & flags) {
			// don't run the call in a transaction
			destroySpecific(TSS_KEY);
		}
		MESSAGE message;
		message.replyto = replyTo;
		message.data = idata;
		message.len = ilen;
		message.flags = flags;
		sender->send(message);
	} catch (...) {
		userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "aCorbaService->start_conversation(): call failed");
		tperrno = TPESYSTEM;
		toReturn = -1;
	}
	return toReturn;
}

int AtmiBrokerConversation::receive(Session* session, char ** odata, long *olen, long flags, long* event) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "tprecv - odata: %s olen: %p flags: %d", *odata, olen, flags);
	int toReturn = 0;
	MESSAGE message = session->getReceiver()->receive(flags);
	if (message.data != NULL) {
		// TODO Handle TPNOCHANGE
		// TODO Handle buffer
		// TODO USE RVAL AND RCODE
		*odata = message.data;
		*olen = message.len;
		*event = message.event;
		session->setReplyTo((char*) message.replyto);
		userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "returning - %s", *odata);
	} else {
		tperrno = TPETIME;
		toReturn = -1;
	}
	return toReturn;
}

int AtmiBrokerConversation::disconnect(int id) {
	userlog(Level::getDebug(), loggerAtmiBrokerConversation, (char*) "end - id: %d", id);

	int toReturn = -1;

	//	EndpointQueue * callback = mAtmiBrokerClient->getRemoteCallback(id);
	//	if (callback == NULL) {
	//		tperrno = TPEBADDESC;
	//	} else {
	//		try {
	//			callback->disconnect();
	//			toReturn = 0;
	//		} catch (...) {
	//			userlog(Level::getError(), loggerAtmiBrokerConversation, (char*) "callback->disconnect(): call failed");
	//			tperrno = TPESYSTEM;
	//		}
	//	}
	return toReturn;
}
