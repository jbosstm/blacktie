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

#include "ace/OS_NS_unistd.h"

#include "ThreadLocalStorage.h"
#include "BaseServerTest.h"
#include "XATMITestSuite.h"

#include "xatmi.h"
#include "tx.h"

#include "TestTxTPCall.h"

/* service routines */
static void tx_fill_buf(TPSVCINFO *svcinfo) {
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);

	strcpy(toReturn, "inTx=");
	strcat(toReturn, inTx ? "true" : "false");

	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tx_tpcall_x_octet_service_tardy(TPSVCINFO *svcinfo) {
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_tardy");
	tx_fill_buf(svcinfo);
	ACE_OS::sleep(6L);
}

void test_tx_tpcall_x_octet_service_without_tx(TPSVCINFO *svcinfo) {
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_without_tx");
	tx_fill_buf(svcinfo);
}

void test_tx_tpcall_x_octet_service_with_tx(TPSVCINFO *svcinfo) {
	userlogc((char*) "TxLog: service running: test_tx_tpcall_x_octet_service_with_tx");
	tx_fill_buf(svcinfo);
}

/* test setup */
void TestTxTPCall::setUp() {
	BaseServerTest::setUp();

	// previous tests may have left a txn on the thread
	destroySpecific(TSS_KEY);
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	sendlen = strlen("TestTxTPCall") + 1;
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	(void) strcpy(sendbuf, "TestTxTPCall");
	rcvlen = 60;
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen)) != NULL);
	CPPUNIT_ASSERT(tperrno == 0);
}

/* test teardown */
void TestTxTPCall::tearDown() {
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	destroySpecific(TSS_KEY);
	int rc = tpunadvertise((char*) "tpcall_x_octet");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);
	CPPUNIT_ASSERT(tx_close() == TX_OK);

	// Clean up server
	BaseServerTest::tearDown();
}

/* client routines */
void TestTxTPCall::test_timeout_no_tx() {
	userlogc((char*) "TxLog: test_timeout_no_tx");
	int rc = tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_tardy);
	CPPUNIT_ASSERT(tperrno == 0 && rc != -1);
	int cd = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "inTx=false") == 0);
}

void TestTxTPCall::test_timeout_with_tx() {
	userlogc((char*) "TxLog: test_timeout_with_tx");
	int rv1 = tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_tardy);
	// the service will sleep for 4 seconds so set the timeout to be less that 4
	int rv2 = tx_set_transaction_timeout(2);
	CPPUNIT_ASSERT(rv1 != 1 && rv2 == TX_OK && tx_begin() == TX_OK);
	int rv3 = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	userlogc((char*) "TxLog: test_timeout_with_tx tpcall=%d tperrno=%d", rv3, tperrno);
	CPPUNIT_ASSERT(rv3 == -1);
	CPPUNIT_ASSERT(tperrno == TPETIME);
	// the transaction should have been marked as rollback only
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestTxTPCall::test_tpcall_without_tx() {
	userlogc((char*) "TxLog: test_tpcall_without_tx");
	int rc = tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_without_tx);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "inTx=false") == 0);
	// make sure there is no active transaction
	CPPUNIT_ASSERT(tx_commit() != TX_OK);
	userlogc_debug((char*) "TxLog: test_tpcall_without_tx: passed");
}

void TestTxTPCall::test_tpcall_with_tx() {
	userlogc((char*) "TxLog: test_tpcall_with_tx");
	int rc = tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_with_tx);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	// start a transaction
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tx_open");
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tpcall");
	(void) ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	userlogc_debug((char*) "TxLog: test_tpcall_with_tx: tx_commit");
	// make sure there is still an active transaction - ie starting a new one should fail
/*	CPPUNIT_ASSERT(tx_begin() != TX_OK);*/
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "inTx=true") == 0);
}

void TestTxTPCall::test_tpcancel_with_tx() {
	userlogc((char*) "TxLog: test_tpcancel_with_tx");
	int rc = tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service_with_tx);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	// start a transaction
	userlogc_debug((char*) "TxLog: test_tpcancel_with_tx: tx_open");
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	userlogc_debug((char*) "TxLog: test_tpcancel_with_tx: tpcall");
	int cd = ::tpacall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (long) 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	// cancel should fail with TPETRAN since the outstanding call is transactional
	userlogc_debug((char*) "TxLog: test_tpcancel_with_tx: tpcancel %d", cd);
	int cancelled = ::tpcancel(cd);
	CPPUNIT_ASSERT(cancelled == -1);
	CPPUNIT_ASSERT(tperrno == TPETRAN);
	// a tpgetrply should succeed since the tpcancel request will have failed
	int res = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(res != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	userlogc_debug((char*) "TxLog: test_tpcancel_with_tx: tx_commit");
	// commit should succeed since the failed tpcancel does not affect the callers tx
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "inTx=true") == 0);
}
