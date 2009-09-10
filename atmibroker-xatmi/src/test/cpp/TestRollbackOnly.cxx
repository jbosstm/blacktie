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

#include "BaseServerTest.h"
#include "XATMITestSuite.h"

#include "xatmi.h"
#include "tx.h"

#include "TestRollbackOnly.h"

extern void test_tpcall_TPETIME_service(TPSVCINFO *svcinfo);
extern void test_tpcall_TPEOTYPE_service(TPSVCINFO *svcinfo);
extern void test_tpcall_TPESVCFAIL_service(TPSVCINFO *svcinfo);
extern void test_tprecv_TPEV_DISCONIMM_service(TPSVCINFO *svcinfo);
extern void test_tprecv_TPEV_SVCFAIL_service(TPSVCINFO *svcinfo);
extern void test_no_tpreturn_service(TPSVCINFO *svcinfo);

void TestRollbackOnly::setUp() {
	BaseServerTest::setUp();

	sendlen = strlen("TestRbkOnly") + 1;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	(void) strcpy(sendbuf, "TestRbkOnly");
	rcvlen = 60;
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestRollbackOnly::tearDown() {
	CPPUNIT_ASSERT(tx_close() == TX_OK);

	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestRollbackOnly::test_tpcall_TPETIME() {
	userlogc((char*) "test_tpcall_TPETIME");
	int rc = tpadvertise((char*) "TestRbkOnly", test_tpcall_TPETIME_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	(void) ::tpcall((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == TPETIME);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestRollbackOnly::test_tpcall_TPEOTYPE() {
	userlogc((char*) "test_tpcall_TPETIME");
	int rc = tpadvertise((char*) "TestRbkOnly", test_tpcall_TPEOTYPE_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	(void) ::tpcall((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, TPNOCHANGE);
	CPPUNIT_ASSERT(tperrno == TPEOTYPE);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestRollbackOnly::test_tpcall_TPESVCFAIL() {
	userlogc((char*) "test_tpcall_TPESVCFAIL");
	int rc = tpadvertise((char*) "TestRbkOnly", test_tpcall_TPESVCFAIL_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	(void) ::tpcall((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf,
			"test_tpcall_TPESVCFAIL_service") == 0);
	CPPUNIT_ASSERT(tperrno == TPESVCFAIL);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestRollbackOnly::test_tprecv_TPEV_DISCONIMM() {
	userlogc((char*) "test_tprecv_TPEV_DISCONIMM");
	int rc = tpadvertise((char*) "TestRbkOnly",
			test_tprecv_TPEV_DISCONIMM_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	int cd = ::tpconnect((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			TPSENDONLY);
	::tpdiscon(cd);
	CPPUNIT_ASSERT(tperrno == 0);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestRollbackOnly::test_tprecv_TPEV_SVCFAIL() {
	userlogc((char*) "test_tprecv_TPEV_SVCFAIL");
	int rc = tpadvertise((char*) "TestRbkOnly",
			test_tprecv_TPEV_SVCFAIL_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	int cd = ::tpconnect((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			TPRECVONLY);
	long revent = 0;
	int status = ::tprecv(cd, (char **) &rcvbuf, &rcvlen, (long) 0, &revent);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf,
			"test_tprecv_TPEV_SVCFAIL_service") == 0);
	CPPUNIT_ASSERT(status == -1);
	CPPUNIT_ASSERT(revent == TPEV_SVCFAIL);
	CPPUNIT_ASSERT(tperrno == TPEEVENT);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

void TestRollbackOnly::test_no_tpreturn() {
	userlogc((char*) "test_no_tpreturn");
	int rc = tpadvertise((char*) "TestRbkOnly", test_no_tpreturn_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);

	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);

	(void) ::tpcall((char*) "TestRbkOnly", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == TPESVCERR);

	TXINFO txinfo;
	int inTx = ::tx_info(&txinfo);
	userlogc((char*) "inTx=%d", inTx);
	CPPUNIT_ASSERT(txinfo.transaction_state == TX_ROLLBACK_ONLY);
	CPPUNIT_ASSERT(tx_commit() == TX_ROLLBACK);
}

