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
#include "Message.h"

LoggerPtr ServiceDispatcher::logger(Logger::getLogger("ServiceDispatcher"));

ServiceDispatcher::ServiceDispatcher(Receiver* serviceQueue, MessageListener* service) :
	m_serviceQueue(serviceQueue), m_service(service), m_shutdown(false) {
}

int ServiceDispatcher::svc(void) {
	while (!m_shutdown) {
		MESSAGE message = m_serviceQueue->receive(0);
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
