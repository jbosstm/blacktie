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
#include <string.h>
#include "SessionImpl.h"
#include "SenderImpl.h"
#include "ReceiverImpl.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(void* connection_poa, void* connection_orb, int id) {
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "constructor ");
	this->id = id;
	this->connection_orb = connection_orb;
	queueReceiver = new ReceiverImpl(connection_poa, connection_orb);
	queueSender = NULL;
}

SessionImpl::~SessionImpl() {
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "destructor");
	if (queueReceiver) {
		delete queueReceiver;
		queueReceiver = NULL;
	}
	if (queueSender) {
		delete queueSender;
		queueSender = NULL;
	}
}

void SessionImpl::setReplyTo(char * replyTo) {
	if (queueSender) {
		delete queueSender;
		queueSender = NULL;
	}
	if (strcmp(replyTo, "") != 0) {
		queueSender = new SenderImpl(createTemporaryQueue(replyTo));
	}
}

Receiver * SessionImpl::getReceiver() {
	return queueReceiver;
}

Sender * SessionImpl::getSender() {
	return queueSender;
}

Destination* SessionImpl::createTemporaryQueue(char* queueName) {
	return new EndpointQueue(connection_orb, queueName);
}
