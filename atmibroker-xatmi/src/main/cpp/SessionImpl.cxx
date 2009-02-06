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

log4cxx::LoggerPtr SessionImpl::logger(log4cxx::Logger::getLogger("SessionImpl"));

SessionImpl::SessionImpl(CONNECTION* connection, Destination* destination, int id) {
	LOG4CXX_LOGLS(logger, log4cxx::Level::getDebug(), (char*) "constructor ");
	this->id = id;
	this->connection = connection;
	queueReceiver = new ReceiverImpl(destination);
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

void SessionImpl::setSendTo(Destination* destination) {
	if (queueSender) {
		delete queueSender;
		queueSender = NULL;
	}
	if (destination != NULL) {
		queueSender = new SenderImpl(destination);
	}
}

Receiver * SessionImpl::getReceiver() {
	return queueReceiver;
}

Sender * SessionImpl::getSender() {
	return queueSender;
}

int SessionImpl::getId() {
	return id;
}
