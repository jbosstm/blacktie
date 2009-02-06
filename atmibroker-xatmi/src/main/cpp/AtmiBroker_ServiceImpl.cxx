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
#include <string.h>
#include "tx.h"
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBroker.h"
#include "SessionImpl.h"
#include "userlog.h"
#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr AtmiBroker_ServiceImpl::logger(log4cxx::Logger::getLogger("AtmiBroker_ServiceImpl"));

// AtmiBroker_ServiceImpl constructor
//
AtmiBroker_ServiceImpl::AtmiBroker_ServiceImpl(CONNECTION* connection, char *serviceName, void(*func)(TPSVCINFO *)) {
	m_connection = connection;
	m_serviceName = serviceName;
	m_func = func;
	session = NULL;
}

// ~AtmiBroker_ServiceImpl destructor.
//
AtmiBroker_ServiceImpl::~AtmiBroker_ServiceImpl() {
	// Intentionally empty.
	//
}

void AtmiBroker_ServiceImpl::onMessage(MESSAGE message) {
	userlog(log4cxx::Level::getDebug(), logger, (char*) "svc()");

	// INITIALISE THE SENDER AND RECEIVER FOR THIS CONVERSATION
	session = new SessionImpl(m_connection, ::create_temporary_queue(m_connection), -1);
	session->setReplyTo(::lookup_temporary_queue(m_connection, (char*) message.replyto));

	// EXTRACT THE DATA FROM THE INBOUND MESSAGE
	int correlationId = message.correlationId;
	char* idata = message.data;
	long ilen = message.len;
	long flags = message.flags;
	void* control = message.control;
	userlog(log4cxx::Level::getDebug(), logger, (char*) "   idata = %p", idata);
	userlog(log4cxx::Level::getDebug(), logger, (char*) "   ilen = %d", ilen);
	userlog(log4cxx::Level::getDebug(), logger, (char*) "   flags = %d", flags);

	// PREPARE THE STRUCT FOR SENDING TO THE CLIENT
	TPSVCINFO tpsvcinfo;
	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, m_serviceName);
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
	setSpecific(TSS_KEY, control);
	setSpecific(SVC_KEY, this);
	setSpecific(SVC_SES, session);
	tx_open();
	m_func(&tpsvcinfo);
	tx_close();
	destroySpecific(SVC_SES);
	destroySpecific(SVC_KEY);
	destroySpecific(TSS_KEY);

	// CLEAN UP THE SENDER AND RECEIVER FOR THIS CLIENT
	if (session) {
		delete session;
		session = NULL;
	}
}

Session* AtmiBroker_ServiceImpl::getSession() {
	return session;
}
