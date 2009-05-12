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
#include "ServiceDispatcher.h"
#include "txClient.h"
#include "ThreadLocalStorage.h"

log4cxx::LoggerPtr ServiceDispatcher::logger(log4cxx::Logger::getLogger("ServiceDispatcher"));

ServiceDispatcher::ServiceDispatcher(Destination* destination, Connection* connection, const char *serviceName, void(*func)(TPSVCINFO *)) {
	this->destination = destination;
	this->connection = connection;
	this->serviceName = serviceName;
	this->func = func;
	session = NULL;
	stop = false;
}

int ServiceDispatcher::svc(void) {
	while (!stop) {
		MESSAGE message = destination->receive(0);
		if (!stop && message.len > -1) {
			try {
				onMessage(message);
			} catch (...) {
				LOG4CXX_ERROR(logger, (char*) "Service Dispatcher caught error running during onMessage");
			}
		}
	}
	return 0;
}

void ServiceDispatcher::onMessage(MESSAGE message) {
	LOG4CXX_DEBUG(logger, (char*) "svc()");

	// INITIALISE THE SENDER AND RECEIVER FOR THIS CONVERSATION
	if (message.replyto) {
		LOG4CXX_DEBUG(logger, (char*) "   replyTo = " << message.replyto);
	} else {
		LOG4CXX_DEBUG(logger, (char*) "   replyTo = NULL");
	}
	session = connection->createSession(message.correlationId, message.replyto);

	// EXTRACT THE DATA FROM THE INBOUND MESSAGE
	int correlationId = message.correlationId;
	
	char* idata = (char *) malloc(message.len);
	memcpy(idata, message.data, message.len);

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

	if (tpsvcinfo.flags & TPRECVONLY) {
		session->setCanRecv(false);
	} else if (tpsvcinfo.flags & TPSENDONLY) {
		session->setCanSend(false);
	}
	// HANDLE THE CLIENT INVOCATION
	if (control) {
		tpsvcinfo.flags = (tpsvcinfo.flags | TPTRAN);
		// TODO wrap TSS control in a Transaction object and make sure any current
		// control associated with the thread is suspended here and resumed after
		// the call to m_func
		associate_tx(control);
	}
	setSpecific(SVC_KEY, this);
	setSpecific(SVC_SES, session);
	try {
		this->func(&tpsvcinfo);
	} catch (...) {
		LOG4CXX_ERROR(logger, (char*) "ServiceDispatcher caught error running during onMessage");
	}

	LOG4CXX_TRACE(logger, (char*) "Freeing the data that was passed to the service");
	free(idata);
	if (control) {
		disassociate_tx(); // TODO figure out why tpreturn needs to stop Resource Managers
		setSpecific(TSS_KEY, control);
	}

	// CLEAN UP THE SENDER AND RECEIVER FOR THIS CLIENT
	if (session->getCanSend()) {
		::tpreturn(TPFAIL, TPESVCERR, NULL, 0, 0);
	}
	delete this->session;
	this->session = NULL;

	destroySpecific(SVC_SES);
	destroySpecific(SVC_KEY);
	destroySpecific(TSS_KEY);
	//disassociateTx(); TODO
	LOG4CXX_DEBUG(logger, (char*) "ServiceDispatcher session closed");
}

void ServiceDispatcher::shutdown() {
	stop = true;
}
