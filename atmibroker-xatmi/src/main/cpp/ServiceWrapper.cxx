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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ServiceWrapper.h"
#include "ThreadLocalStorage.h"
#include "AtmiBrokerOTS.h"

log4cxx::LoggerPtr ServiceWrapper::logger(log4cxx::Logger::getLogger("ServiceWrapper"));

// ServiceWrapper constructor
//
ServiceWrapper::ServiceWrapper(Connection* connection, char *serviceName, void(*func)(TPSVCINFO *)) {
	this->connection = connection;
	this->serviceName = serviceName;
	this->func = func;
	session = NULL;
}

// ~ServiceWrapper destructor.
//
ServiceWrapper::~ServiceWrapper() {
	// Intentionally empty.
	//
}

void ServiceWrapper::onMessage(MESSAGE message) {
	LOG4CXX_DEBUG(logger, (char*) "svc()");

	// INITIALISE THE SENDER AND RECEIVER FOR THIS CONVERSATION
	session = connection->createSession();
	if (message.replyto) {
		LOG4CXX_DEBUG(logger, (char*) "   replyTo = " << message.replyto);
	} else {
		LOG4CXX_DEBUG(logger, (char*) "   replyTo = NULL");
	}
	session->setSendTo((char*) message.replyto);

	// EXTRACT THE DATA FROM THE INBOUND MESSAGE
	int correlationId = message.correlationId;
	char* idata = message.data;
	long ilen = message.len;
	long flags = message.flags;
	void* control = message.control;
	LOG4CXX_DEBUG(logger, (char*) "   idata = %p" << idata);
	LOG4CXX_DEBUG(logger, (char*) "   ilen = %d" << ilen);
	LOG4CXX_DEBUG(logger, (char*) "   flags = %d" << flags);

	// PREPARE THE STRUCT FOR SENDING TO THE CLIENT
	TPSVCINFO tpsvcinfo;
	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, this->serviceName);
	tpsvcinfo.flags = flags;
	tpsvcinfo.data = idata;
	tpsvcinfo.len = ilen;
	if (tpsvcinfo.flags & TPCONV) {
		tpsvcinfo.cd = correlationId;
	}
	if (control) {
		tpsvcinfo.flags = (tpsvcinfo.flags | TPTRAN);
	}

	// HANDLE THE CLIENT INVOCATION
	// TODO wrap TSS control in a Transaction object and make sure any current
	// control associated with the thread is suspended here and resumed after
	// the call to m_func
	setSpecific(TSS_KEY, control);
	setSpecific(SVC_KEY, this);
	setSpecific(SVC_SES, session);
	AtmiBrokerOTS::get_instance()->rm_resume();
	try {
		this->func(&tpsvcinfo);
	} catch (...) {
		LOG4CXX_ERROR(logger, (char*) "Service Wrapper caught error running during onMessage");
	}
	AtmiBrokerOTS::get_instance()->rm_suspend();
	destroySpecific(SVC_SES);
	destroySpecific(SVC_KEY);
	destroySpecific(TSS_KEY);

	// CLEAN UP THE SENDER AND RECEIVER FOR THIS CLIENT
	if (session) {
		delete this->session;
		this->session = NULL;
		LOG4CXX_DEBUG(logger, (char*) "ServiceWrapper session closed");
	}
}

Session* ServiceWrapper::getSession() {
	return session;
}
