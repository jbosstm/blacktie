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
#include "txx.h"
#include "ThreadLocalStorage.h"
#include "AtmiBrokerEnv.h"
#include "AtmiBrokerMem.h"
#include "txx.h"
#include <tao/ORB.h>

log4cxx::LoggerPtr ServiceDispatcher::logger(log4cxx::Logger::getLogger(
		"ServiceDispatcher"));

ServiceDispatcher::ServiceDispatcher(Destination* destination,
		Connection* connection, const char *serviceName, void(*func)(
				TPSVCINFO *), bool isPause) {
	this->destination = destination;
	this->connection = connection;
	this->serviceName = strdup(serviceName);
	this->func = func;
	this->isPause = isPause;
	session = NULL;
	stop = false;
	this->timeout = (long) (mqConfig.destinationTimeout * 1000000);
	this->counter = 0;
}

ServiceDispatcher::~ServiceDispatcher() {
	free(this->serviceName);
}

int ServiceDispatcher::pause(void) {
	LOG4CXX_TRACE(logger, "ServiceDispatcher pause");
	if (isPause == false) {
		isPause = true;
	}
	return 0;
}

int ServiceDispatcher::resume(void) {
	LOG4CXX_TRACE(logger, "ServiceDispatcher resume");
	if (isPause) {
		isPause = false;
	}
	return 0;
}

int ServiceDispatcher::svc(void) {
	while (!stop) {
		MESSAGE message = destination->receive(this->timeout);
		if (!isPause && !stop && message.received) {
			try {
				counter += 1;
				onMessage(message);
			} catch (const CORBA::BAD_PARAM& ex) {
				LOG4CXX_WARN(logger, (char*) "Service dispatcher BAD_PARAM: "
						<< ex._name());
			} catch (const CORBA::SystemException& ex) {
				LOG4CXX_WARN(logger,
						(char*) "Service dispatcher SystemException: "
								<< ex._name());
			} catch (...) {
				LOG4CXX_ERROR(
						logger,
						(char*) "Service Dispatcher caught error running during onMessage");
			}
		} else if (tperrno == TPESYSTEM) {
			LOG4CXX_WARN(
					logger,
					(char*) "Service dispatcher detected dead connection, attempting destination reconnect");
		}
	}
	return 0;
}

void ServiceDispatcher::onMessage(MESSAGE message) {
	setSpecific(TPE_KEY, TSS_TPERESET);

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

	int correlationId = message.correlationId;
	long ilen = message.len;
	long flags = message.flags;

	LOG4CXX_DEBUG(logger, (char*) "ilen: " << ilen << " flags: " << flags
			<< "cd: " << message.correlationId << " message.control="
			<< message.control);

	// PREPARE THE STRUCT FOR SENDING TO THE CLIENT
	TPSVCINFO tpsvcinfo;
	memcpy(tpsvcinfo.name, message.serviceName, XATMI_SERVICE_NAME_LENGTH);
	memset(&tpsvcinfo, '\0', sizeof(tpsvcinfo));
	strcpy(tpsvcinfo.name, this->serviceName);
	tpsvcinfo.flags = flags;
	tpsvcinfo.len = ilen;

	if (message.data != NULL) {
		tpsvcinfo.data = AtmiBrokerMem::get_instance()->tpalloc(message.type,
				message.subtype, ilen, true);
		if (message.len > 0) {
			memcpy(tpsvcinfo.data, message.data, ilen);
		}
		free(message.data);
	} else {
		tpsvcinfo.data = NULL;
	}

	setSpecific(SVC_KEY, this);
	setSpecific(SVC_SES, session);

	if (tpsvcinfo.flags & TPCONV) {
		tpsvcinfo.cd = correlationId;
		long olen = 4;
		char* odata = (char*) tpalloc((char*) "X_OCTET", NULL, olen);
		strcpy(odata, "ACK");
		long revent = 0;
		long result = tpsend(tpsvcinfo.cd, odata, olen, 0, &revent);
		if (result == -1) {
			connection->closeSession(message.correlationId);
			destroySpecific( SVC_SES);
			destroySpecific( SVC_KEY);
			return;
		}

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
	if (message.control != NULL && strcmp((char*) message.control, "null") != 0) {
		if (txx_associate_serialized((char*) message.control) != XA_OK) {
			LOG4CXX_ERROR(logger, "Unable to handle control");
			setSpecific(TPE_KEY, TSS_TPESYSTEM);
		}
		tpsvcinfo.flags = (tpsvcinfo.flags | TPTRAN);
	}

	if (tperrno == 0) {
		try {
			LOG4CXX_TRACE(logger, (char*) "Calling function");
			this->func(&tpsvcinfo);
			LOG4CXX_TRACE(logger, (char*) "Called function");
		} catch (...) {
			LOG4CXX_ERROR(
					logger,
					(char*) "ServiceDispatcher caught error running during onMessage");
		}
	} else {
		LOG4CXX_ERROR(logger,
				(char*) "Not invoking tpservice as tpernno was not 0");
	}

	AtmiBrokerMem::get_instance()->tpfree(tpsvcinfo.data, true);

	// CLEAN UP THE SENDER AND RECEIVER FOR THIS CLIENT
	if (session->getCanSend()) {
		LOG4CXX_TRACE(logger,
				(char*) "Returning error - marking tx as rollback only if "
						<< getSpecific(TSS_KEY));
		::tpreturn(TPFAIL, TPESVCERR, NULL, 0, 0);
		LOG4CXX_TRACE(logger, (char*) "Returned error");
	} else if (getSpecific(TSS_KEY) != NULL) {
		txx_release_control(txx_unbind(true));
	}

	LOG4CXX_TRACE(logger, (char*) "ServiceDispatcher closing session: "
			<< message.correlationId);
	connection->closeSession(message.correlationId);
	//	session = NULL;
	LOG4CXX_TRACE(logger, (char*) "ServiceDispatcher session closed: "
			<< message.correlationId);
	//	HybridConnectionImpl* instance = dynamic_cast<HybridConnectionImpl*> (connection);
	//shutdownBindings(instance->connection);

	destroySpecific( SVC_SES);
	destroySpecific( SVC_KEY);

	LOG4CXX_TRACE(logger,
			(char*) "Freeing the data that was passed to the service");
	//	free(idata);
	LOG4CXX_TRACE(logger, (char*) "Freed the data");
}

void ServiceDispatcher::shutdown() {
	stop = true;
}

long ServiceDispatcher::getCounter() {
	return counter;
}
