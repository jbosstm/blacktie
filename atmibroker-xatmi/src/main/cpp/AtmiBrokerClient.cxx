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

#include "log4cxx/logger.h"

#include "userlog.h"
#include "AtmiBroker.h"
#include "AtmiBrokerClient.h"
#include "SessionImpl.h"

log4cxx::LoggerPtr loggerAtmiBrokerClient(log4cxx::Logger::getLogger("AtmiBrokerClient"));

AtmiBrokerClient::AtmiBrokerClient() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClient, (char*) "constructor ");

	AtmiBrokerClientXml aAtmiBrokerClientXml;
	aAtmiBrokerClientXml.parseXmlDescriptor(&clientServerVector, "CLIENT.xml");

	session = NULL;
}

AtmiBrokerClient::~AtmiBrokerClient() {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClient, (char*) "destructor ");

	for (std::vector<ClientServerInfo*>::iterator itServer = clientServerVector.begin(); itServer != clientServerVector.end(); itServer++) {
		userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerClient, (char*) "next serverName is: %s", (char*) (*itServer)->serverName);
	}
	clientServerVector.clear();
}

Session* AtmiBrokerClient::createSession(int& id) {
	if (session) {
		delete session;
		session = NULL;
	}
	id = 0;
	session = new SessionImpl(clientConnection, 0);
	return session;
}

Session* AtmiBrokerClient::getSession(int* id) {
	return session;
}

void AtmiBrokerClient::closeSession(int id) {
	if (session) {
		delete session;
		session = NULL;
	}
}
