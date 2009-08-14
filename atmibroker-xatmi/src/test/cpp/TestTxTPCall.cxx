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

#include "BaseServerTest.h"
#include "XATMITestSuite.h"

#include "xatmi.h"
#include "tx.h"

#include "TestTxTPCall.h"

extern void test_tx_tpcall_x_octet_service_without_tx(TPSVCINFO *svcinfo);
extern void test_tx_tpcall_x_octet_service_with_tx(TPSVCINFO *svcinfo);

void TestTxTPCall::setUp() {
	BaseServerTest::setUp();

	sendlen = strlen("TestTxTPCall") + 1;
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	(void) strcpy(sendbuf, "TestTxTPCall");
	rcvlen = 60;
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen)) != NULL);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTxTPCall::tearDown() {
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTxTPCall::test_tpcall_without_tx() {
	userlogc((char*) "TxLog: test_tpcall_without_tx");
	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_without_tx);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "test_tx_tpcall_x_octet_service_without_tx") == 0);
	// make sure there is no active transaction
	CPPUNIT_ASSERT(tx_commit() != TX_OK);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
	userlogc_debug((char*) "TxLog: test_tpcall_without_tx: passed");
}

void TestTxTPCall::test_tpcall_with_tx() {
	userlogc((char*) "TxLog: test_tpcall_with_tx");

	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_with_tx);

	// start a transaction
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tx_open");
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tpcall");
	(void) ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tx_commit");
	// make sure there is still an active transaction - ie starting a new one should fail
/*	CPPUNIT_ASSERT(tx_begin() != TX_OK);*/
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "test_tx_tpcall_x_octet_service_with_tx") == 0);
}

/* service routines */
void test_tx_tpcall_x_octet_service_without_tx(TPSVCINFO *svcinfo) {
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_without_tx");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	TXINFO txinfo;
    int inTx = ::tx_info(&txinfo);
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_without_tx inTx=%d", inTx);
	if (inTx == 0) { // or && txinfo.transaction_state != TX_ACTIVE
		strcpy(toReturn, "test_tx_tpcall_x_octet_service_without_tx");
	} else {
		strcpy(toReturn, svcinfo->data);
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tx_tpcall_x_octet_service_with_tx(TPSVCINFO *svcinfo) {
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_with_tx");
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	TXINFO txinfo;
    int inTx = ::tx_info(&txinfo);
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_with_tx inTx=%d", inTx);
	if (inTx == 1) { // or && txinfo.transaction_state == TX_ACTIVE
		strcpy(toReturn, "test_tx_tpcall_x_octet_service_with_tx");
	} else {
		strcpy(toReturn, svcinfo->data);
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
