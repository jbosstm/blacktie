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
#include "TestAssert.h"

#include "TestXAStompConnection.h"

#include "userlogc.h"
#include "AtmiBrokerEnv.h"
#include "tx.h"
#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_TX_DLL char* txx_serialize(long* ttl);
#ifdef __cplusplus
}
#endif

void messagesAvailableCallbackXA(int bar, bool remove) {

}

void TestXAStompConnection::setUp() {
	init_ace();
	userlogc("TestXAStompConnection::setUp");

	serverConnection = NULL;
	clientConnection = NULL;
	serverConnection = new HybridConnectionImpl((char*) "server",
			messagesAvailableCallbackXA);
	clientConnection = new HybridConnectionImpl((char*) "client",
			messagesAvailableCallbackXA);
}

void TestXAStompConnection::tearDown() {
	userlogc("TestXAStompConnection::tearDown");
	if (serverConnection) {
		delete serverConnection;
	}
	if (clientConnection) {
		delete clientConnection;
	}
}

void TestXAStompConnection::test() {
	userlogc("TestStompConnection::test");

	int msgCount = 2;

	Session* client = clientConnection->getQueueSession();

	tx_open();
	tx_begin();

	// THIS IS THE INITIAL SEND
	for (int i = 0; i < msgCount; i++) {
		MESSAGE clientSend;
		char* clientData = (char*) malloc(6);
		memset(clientData, '\0', 6);
		strcpy(clientData, "hello");
		clientSend.data = clientData;
		clientSend.correlationId = 0;
		clientSend.flags = 0;
		clientSend.len = 5;
		clientSend.priority = 0;
		clientSend.rval = 0;
		clientSend.rcode = 0;
		clientSend.replyto = client->getReplyTo();
		clientSend.type = (char*) "X_OCTET";
		clientSend.subtype = (char*) "";
		clientSend.ttl = 10 * 1000;
		clientSend.control = NULL;
		long discardTxTTL = -1;
		clientSend.xid = txx_serialize(&discardTxTTL);;
		BT_ASSERT(client->send((char*) "JAVA_Converse", clientSend));
	}

	tx_commit();
	tx_close();

	// THIS IS THE RECEIVE
	Destination* destination = serverConnection->createDestination(
			(char*) "JAVA_Converse", false);

	userlogc("Iterating");
	for (int i = 0; i < msgCount; i++) {
		MESSAGE serviceReceived = destination->receive(0);
		BT_ASSERT(serviceReceived.received);
		destination->ack(serviceReceived);
		free(serviceReceived.data);
	}

	userlogc("Iterated");
	serverConnection->destroyDestination(destination);
}
