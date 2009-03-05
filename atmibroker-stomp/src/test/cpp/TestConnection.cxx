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

#include "userlog.h"
#include "ConnectionImpl.h"

void TestConnection::test() {
	initializeLogger();
	ConnectionImpl* serverConnection = new ConnectionImpl("server");
	ConnectionImpl* clientConnection = new ConnectionImpl("client");
	Destination* destination = serverConnection->createDestination((char*) "LOOPY");
	Session* client = clientConnection->createSession(1, (char*) "LOOPY");
	MESSAGE clientSend;
	clientSend.data = (char*) "hello";
	clientSend.len = 6;
	clientSend.replyto = client->getReplyTo();
	client->send(clientSend);
	MESSAGE clientSent = destination->receive(0);
	Session* service = serverConnection->createSession(1, clientSent.replyto);
	MESSAGE serviceSend;
	serviceSend.data = (char*) "bye";
	serviceSend.len = 4;
	serviceSend.replyto = NULL;
	service->send(serviceSend);
	MESSAGE clientReceived = client->receive(0);
//	delete serverConnection;
//	delete clientConnection;
	CPPUNIT_ASSERT(strcmp(serviceSend.data, clientReceived.data) == 0);
}
