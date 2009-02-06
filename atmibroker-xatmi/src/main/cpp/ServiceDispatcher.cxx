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
#include "ServiceDispatcher.h"
#include "AtmiBroker_ServiceImpl.h"
#include "Message.h"

log4cxx::LoggerPtr ServiceDispatcher::logger(log4cxx::Logger::getLogger("ServiceDispatcher"));

ServiceDispatcher::ServiceDispatcher(CONNECTION* connection, Destination* serviceQueue, char *serviceName, void(*func)(TPSVCINFO *)) :
	m_serviceQueue(serviceQueue), m_shutdown(false) {
	m_service = new AtmiBroker_ServiceImpl(connection, serviceName, func);
}

int ServiceDispatcher::svc(void) {
	while (!m_shutdown) {
		MESSAGE message = m_serviceQueue->receive(false);
		if (!m_shutdown) {
			try {
				m_service->onMessage(message);
			} catch (...) {
				LOG4CXX_ERROR(logger, (char*) "Service Dispatcher caught error running during onMessage");
			}
		}
	}
	return 0;
}

void ServiceDispatcher::shutdown() {
	m_shutdown = true;
}
