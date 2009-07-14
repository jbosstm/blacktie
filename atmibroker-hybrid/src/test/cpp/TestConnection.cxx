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
#include <cppunit/extensions/HelperMacros.h>

#include "TestConnection.h"

#include "userlogc.h"

void TestConnection::setUp() {
	userlogc("TestConnection::setUp");
	serverConnection = NULL;
	clientConnection = NULL;
	serverConnection = new HybridConnectionImpl((char*) "server");
	clientConnection = new HybridConnectionImpl((char*) "client");
}

void TestConnection::tearDown() {
	userlogc("TestConnection::tearDown");
	if (serverConnection) {
		delete serverConnection;
	}
	if (clientConnection) {
		delete clientConnection;
	}
}

void TestConnection::test() {
	userlogc("TestConnection::test");

	Destination* destination = serverConnection->createDestination(
			(char*) "JAVA_Converse");
	Session* client = clientConnection->createSession(1, (char*) "JAVA_Converse");
	MESSAGE clientSend;
	char* clientData = (char*) malloc(6);
	memset(clientData, '\0', 6);
	strcpy(clientData, "hello");
	clientSend.data = clientData;
	clientSend.len = 6;
	clientSend.replyto = client->getReplyTo();
	client->send(clientSend);
	MESSAGE serviceReceived = destination->receive(0);
	CPPUNIT_ASSERT(clientSend.len == serviceReceived.len);

	Session* service = serverConnection->createSession(1,
			serviceReceived.replyto);
	userlogc("Iterating");
	for (int i = 0; i < 100; i++) {
		MESSAGE serviceSend;
		char* serviceData = (char*) malloc(4);
		memset(serviceData, '\0', 4);
		strcpy(serviceData, "bye");
		serviceSend.data = serviceData;
		serviceSend.len = 4;
		serviceSend.replyto = NULL;
		service->send(serviceSend);
		MESSAGE clientReceived = client->receive(0);
		CPPUNIT_ASSERT(serviceSend.len == clientReceived.len);
	}
	userlogc("Iterated");
}
