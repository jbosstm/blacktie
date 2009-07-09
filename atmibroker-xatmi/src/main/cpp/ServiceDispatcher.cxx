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

log4cxx::LoggerPtr ServiceDispatcher::logger(log4cxx::Logger::getLogger(
		"ServiceDispatcher"));

ServiceDispatcher::ServiceDispatcher(Destination* destination,
		Connection* connection, const char *serviceName, void(*func)(
				TPSVCINFO *)) {
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
		message.len = message.len - 1;
		if (!stop && message.len > -1) {
			try {
				onMessage(message);
			} catch (...) {
				LOG4CXX_ERROR(
						logger,
						(char*) "Service Dispatcher caught error running during onMessage");
			}
		}
	}
	return 0;
}

void ServiceDispatcher::onMessage(MESSAGE message) {
	LOG4CXX_DEBUG(logger, (char*) "svc()");

	// INITIALISE THE SENDER AND RECEIVER FOR THIS CONVERSATION
	if (message.replyto) {
		LOG4CXX_DEBUG(logger, (char*) "replyTo: " << message.replyto);
	} else {
		LOG4CXX_DEBUG(logger, (char*) "replyTo: NULL");
	}
	LOG4CXX_TRACE(logger, (char*) "Creating session: " << message.correlationId);
	session = connection->createSession(message.correlationId, message.replyto);
	LOG4CXX_TRACE(logger, (char*) "Created session: " << message.correlationId);

	// EXTRACT THE DATA FROM THE INBOUND MESSAGE

	char* idata = (char *) malloc(message.len);
	memcpy(idata, message.data, message.len);
	int correlationId = message.correlationId;
	long ilen = message.len;
	long flags = message.flags;
	void* control = message.control;
	LOG4CXX_DEBUG(logger, (char*) "ilen: " << ilen << " flags: " << flags
			<< "cd: " << message.correlationId);

	// PREPARE THE STRUCT FOR SENDING TO THE CLIENT
	TPSVCINFO tpsvcinfo;
	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, this->serviceName);
	tpsvcinfo.flags = flags;
	tpsvcinfo.data = idata;
	tpsvcinfo.len = ilen;
	if (tpsvcinfo.flags && TPCONV) {
		tpsvcinfo.cd = correlationId;
	} else {
		LOG4CXX_DEBUG(logger, (char*) "cd not being set");
	}

	if (tpsvcinfo.flags & TPRECVONLY) {
		session->setCanRecv(false);
		LOG4CXX_DEBUG(logger, (char*) "onMessage set constraints session: "
				<< session->getId() << " send(not changed): "
				<< session->getCanSend() << " recv: " << session->getCanRecv());
	} else if (tpsvcinfo.flags & TPSENDONLY) {
		session->setCanSend(false);
		LOG4CXX_DEBUG(logger, (char*) "onMessage set constraints session: "
				<< session->getId() << " send: " << session->getCanSend()
				<< " recv (not changed): " << session->getCanRecv());
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
		LOG4CXX_TRACE(logger, (char*) "Calling function");
		this->func(&tpsvcinfo);
		LOG4CXX_TRACE(logger, (char*) "Called function");
	} catch (...) {
		LOG4CXX_ERROR(
				logger,
				(char*) "ServiceDispatcher caught error running during onMessage");
	}

	if (control) {
		disassociate_tx(); // TODO figure out why tpreturn needs to stop Resource Managers
	}

	// CLEAN UP THE SENDER AND RECEIVER FOR THIS CLIENT
	if (session->getCanSend()) {
		LOG4CXX_TRACE(logger, (char*) "Returning error");
		::tpreturn(TPFAIL, TPESVCERR, NULL, 0, 0);
		LOG4CXX_TRACE(logger, (char*) "Returned error");
	}
	LOG4CXX_TRACE(logger, (char*) "ServiceDispatcher closing session");
	delete this->session;
	this->session = NULL;
	LOG4CXX_TRACE(logger, (char*) "ServiceDispatcher session closed");

	destroySpecific(SVC_SES);
	destroySpecific(SVC_KEY);
	destroySpecific(TSS_KEY);

	LOG4CXX_TRACE(logger,
			(char*) "Freeing the data that was passed to the service");
	free(idata);
	LOG4CXX_TRACE(logger, (char*) "Freed the data");
}

void ServiceDispatcher::shutdown() {
	stop = true;
}
