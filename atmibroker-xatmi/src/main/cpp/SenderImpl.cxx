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
#include "userlog.h"
#include "xatmi.h"
#include "SenderImpl.h"
#include "Message.h"
#include "EndpointQueue.h"

log4cxx::LoggerPtr SenderImpl::logger(log4cxx::Logger::getLogger("SenderImpl"));

SenderImpl::SenderImpl(void* connection_orb, char * callback_ior) {
	destination = new EndpointQueue(connection_orb, callback_ior);
}

SenderImpl::SenderImpl(void* connection_context, void* connection_name_context, const char * serviceName) {
	destination = new EndpointQueue(connection_context, connection_name_context, serviceName);
}

SenderImpl::~SenderImpl() {
}

void SenderImpl::send(MESSAGE message) {
	int data_size = ::tptypes(message.data, NULL, NULL);
	if (data_size >= 0) {
		if (message.len <= 0 || message.len > data_size) {
			message.len = data_size;
		}
		destination->send(message);
		userlog(log4cxx::Level::getDebug(), logger, (char*) "Called back ");
	} else {
		tperrno = TPEINVAL;
		userlog(log4cxx::Level::getError(), logger, (char*) "A NON-BUFFER WAS ATTEMPTED TO BE SENT");
	}
}

void SenderImpl::disconnect() {
	destination->disconnect();
}
