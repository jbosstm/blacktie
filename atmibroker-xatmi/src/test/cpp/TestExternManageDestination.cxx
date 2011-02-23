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
#include "TestExternManageDestination.h"
#include "BaseServerTest.h"

#include "AtmiBrokerServerControl.h"

#include "xatmi.h"
#include "tx.h"
#include "btxatmi.h"
#include "malloc.h"

#include "ace/OS_NS_unistd.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"

#include "Sleeper.h"

#include "SynchronizableObject.h"

#if defined(__cplusplus)
extern "C" {
#endif
extern void test_extern_service(TPSVCINFO *svcinfo);
extern void qservice(TPSVCINFO *svcinfo);
#if defined(__cplusplus)
}
#endif

//static SynchronizableObject* lock = new SynchronizableObject();

void TestExternManageDestination::setUp() {
	userlogc((char*) "TestExternManageDestination::setUp");

	// Set up server
	BaseServerTest::setUp();
}

void TestExternManageDestination::tearDown() {
	userlogc((char*) "TestExternManageDestination::tearDown");

	// Clean up server
	BaseServerTest::tearDown();
}

void TestExternManageDestination::test_tpcall_with_service() {
	userlogc((char*) "test_tpcall_with_service");

	// Advertise the service
	int toCheck = tpadvertise((char*) "TestOne", test_extern_service);
	userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	char msg[80];
	char* buf = (char*) "test";
	long sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long recvlen = 1;

	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen,
			(char**) &recvbuf, &recvlen, 0);

	sprintf(msg, "%d %d (%s)", tperrno, cd, recvbuf);
	BT_ASSERT_MESSAGE(msg, cd != -1);
	BT_ASSERT_MESSAGE(msg, recvlen == 8);
	BT_ASSERT_MESSAGE(msg, strncmp(recvbuf, "testone", 7) == 0);

	tpfree(sendbuf);
	tpfree(recvbuf);
}

void TestExternManageDestination::test_tpcall_without_service() {
	userlogc((char*) "test_tpcall_without_service");

	char* buf = (char*) "test";
	long sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long recvlen = 1;

	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen,
			(char**) &recvbuf, &recvlen, 0);
	userlogc((char*) "test_tpcall_without_service %d %d", cd, tperrno);
	BT_ASSERT(cd == -1);
	/*
	 * We don't return TPENOENT since we allow queuing even if the service is temporarily down.
	 */
	BT_ASSERT(tperrno == TPETIME);
}

static void send_one(int id, int pri) {
	msg_opts_t mopts = {0, 0L, 0};
	char msg[16];
	char* buf;
	long len;
	int cd;

	mopts.priority = pri;
	sprintf(msg, (char*) "%d", id);
	len = strlen(msg) + 1;

	buf = tpalloc((char*) "X_OCTET", NULL, len);

	(void) strcpy(buf, msg);
	cd = btenqueue((char*) "TestOne", &mopts, buf, len, 0);

	BT_ASSERT(tperrno == 0 && cd == 0);

	userlogc("Sent %d %d", id, pri);

	tpfree(buf);
}

static void recv_one(msg_opts_t *mopts, long len, long flags, int expect, int expected_tperrno) {
	char* data = (char*) tpalloc((char*) "X_OCTET", NULL, len);
	int toCheck = btdequeue((char*) "TestOne", mopts, &data, &len, 0L);

	userlogc((char*) "recv_one: tperrno=%d expected_tperrno=%d toCheck=%d",
		tperrno, expected_tperrno, toCheck);
	BT_ASSERT(tperrno == expected_tperrno);
	if (tperrno == 0) {
		BT_ASSERT(toCheck != -1);
		if (expect >= 0)
			BT_ASSERT(atoi(data) == expect);
	}
}

void TestExternManageDestination::test_stored_messages() {
	int i;
	msg_opts_t mopts = {0, 0L, 0};

	userlogc((char*) "test_stored_messages");
	for (i = 30; i < 40; i++)
		send_one(i, 0);

	// retrieve the messages in two goes:
	for (i = 30; i < 40; i++) {
		userlogc((char*) "test_stored_messages: retrieving 5");

		char* data = (char*) tpalloc((char*) "X_OCTET", NULL, 2);
		long len = 2;
		long flags = 0;
		int toCheck = btdequeue((char*) "TestOne", &mopts, &data, &len, flags);
		BT_ASSERT(tperrno == 0 && toCheck != -1);

		int id = atoi(data);
		userlogc((char*) "qservice expected: %d received: %d", i, id);
		

		if (i % 5 == 0) {
			userlogc((char*) "Restart server");
			serverdone();
			startServer();
		}
	}

	userlogc((char*) "test_stored_message passed");
}

void TestExternManageDestination::test_stored_message_priority() {
	userlogc((char*) "test_stored_message_priority");
	// send messages with out of order ids - the qservice should receive them in order
	int prefix = 70;
	msg_opts_t mopts = {0, 0L, 0};

	send_one(prefix + 8, 1);
	send_one(prefix + 6, 3);
	send_one(prefix + 4, 5);
	send_one(prefix + 2, 7);
	send_one(prefix + 0, 9);
	send_one(prefix + 9, 0);
	send_one(prefix + 7, 2);
	send_one(prefix + 5, 4);
	send_one(prefix + 3, 6);
	send_one(prefix + 1, 8);

	int msgId = prefix;

	// retrieve the messages in two goes:

	for (int msgCnt = 0; msgCnt < 10; msgCnt++) {
		char* data = (char*) tpalloc((char*) "X_OCTET", NULL, 2);
		long len = 2;
		long flags = 0;
		int toCheck = btdequeue((char*) "TestOne", &mopts, &data, &len, flags);
		BT_ASSERT(tperrno == 0 && toCheck != -1);
		
		char msg[80];
		int id = atoi(data);

		userlogc((char*) "qservice iteration: %d expected: %d received: %d", msgCnt, msgId, id);
		sprintf(msg, "Out of order messages: %d %d", msgId, id);
		BT_ASSERT_MESSAGE(msg, msgId == id);

		msgId += 1;
	}
	serverdone();

	userlogc((char*) "test_stored_message_priority passed");
}

void TestExternManageDestination::test_btenqueue_with_txn_abort() {
	int i;
	msg_opts_t mopts = {0, 500L, 1};

	userlogc((char*) "test_btenqueue_with_txn_abort");
	BT_ASSERT(tx_open() == TX_OK);
	BT_ASSERT(tx_begin() == TX_OK);

	// enqueue messages within a transaction but then abort it
	for (i = 30; i < 40; i++)
		send_one(i, 0);

	BT_ASSERT(tx_rollback() == TX_OK);

	// since the txn aborted the queue will be empty and btdequeue should fail with TPETIME
	userlogc((char*) "testing that btdequeue returns TPETIME");
	recv_one(&mopts, 2L, 0L, i, TPETIME);

	BT_ASSERT(tx_close() == TX_OK);

	userlogc((char*) "test_btenqueue_with_txn_abort passed");
}

void TestExternManageDestination::test_btenqueue_with_txn_commit() {
	int i;
	msg_opts_t mopts = {0, 0L, 1};

	userlogc((char*) "test_btenqueue_with_txn_commit");
	BT_ASSERT(tx_open() == TX_OK);
	BT_ASSERT(tx_begin() == TX_OK);

	// enqueue messages within a transaction and then commit it
	for (i = 30; i < 40; i++)
		send_one(i, 0);

	BT_ASSERT(tx_commit() == TX_OK);

	// since the txn commited btdequeue should retrieve them all
	for (i = 30; i < 40; i++)
		recv_one(&mopts, 2L, 0L, i, 0);

	BT_ASSERT(tx_close() == TX_OK);

	userlogc((char*) "test_btenqueue_with_txn_commit passed");
}

void TestExternManageDestination::test_btdequeue_with_txn_abort() {
	msg_opts_t mopts = {0, 0L, 1};
	int i;

	userlogc((char*) "test_btdequeue_with_txn_abort");
	BT_ASSERT(tx_open() == TX_OK);

	// enqueue messages
	for (i = 30; i < 40; i++)
		send_one(i, 0);

	// dequeue messages within a transaction and then abort it
	BT_ASSERT(tx_begin() == TX_OK);
	for (i = 30; i < 40; i++)
		recv_one(&mopts, 2L, 0L, i, 0);

	BT_ASSERT(tx_rollback() == TX_OK);

	// since the txn was abort the queue will still contains the messages
	for (i = 30; i < 40; i++)
		recv_one(&mopts, 2L, 0L, i, 0);

	BT_ASSERT(tx_close() == TX_OK);

	userlogc((char*) "test_btdequeue_with_txn_abort passed");
}

void TestExternManageDestination::test_btdequeue_with_txn_commit() {
	msg_opts_t mopts = {0, 0L, 1};
	int i;

	userlogc((char*) "test_btdequeue_with_txn_commit");
	BT_ASSERT(tx_open() == TX_OK);

	// enqueue messages
	for (i = 30; i < 40; i++)
		send_one(i, 0);

	// and dequeue them within a transaction and then commit it
	BT_ASSERT(tx_begin() == TX_OK);
	for (i = 30; i < 40; i++)
		recv_one(&mopts, 2L, 0L, i, 0);

	BT_ASSERT(tx_commit() == TX_OK);

	// test that all the messages were dequeued
	userlogc((char*) "testing that btdequeue returns TPETIME");
	mopts.ttl = 500L;
	recv_one(&mopts, 2L, 0L, i, TPETIME);

	BT_ASSERT(tx_close() == TX_OK);

	userlogc((char*) "test_btdequeue_with_txn_commit passed");
}

void test_extern_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_extern_service");
	int len = 8;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "testone");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
