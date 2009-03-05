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

extern void test_tx_tpcall_x_octet_service(TPSVCINFO *svcinfo);

void TestTxTPCall::setUp() {
	BaseServerTest::setUp();

	// Do local work
	sendlen = strlen("hello") + 1;
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	(void) strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTxTPCall::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	// Clean up server
	BaseServerTest::tearDown();
}

void TestTxTPCall::test_tpcall_without_tx() {
	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_octet") == 0);
	// make sure there is no active transaction
	CPPUNIT_ASSERT(tx_commit() != TX_OK);
}

void TestTxTPCall::test_tpcall_with_tx() {

	tpadvertise((char*) "tpcall_x_octet", test_tx_tpcall_x_octet_service);

	// start a transaction
	CPPUNIT_ASSERT(tx_open() == TX_OK);
	CPPUNIT_ASSERT(tx_begin() == TX_OK);
	(void) ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	// make sure there is still an active transaction - ie starting a new one should fail
	CPPUNIT_ASSERT(tx_begin() != TX_OK);
	CPPUNIT_ASSERT(tx_commit() == TX_OK);
	CPPUNIT_ASSERT(tx_close() == TX_OK);
}

void test_tx_tpcall_x_octet_service(TPSVCINFO *svcinfo) {
	bool ok = false;
	if (strncmp(svcinfo->data, "hello", svcinfo->len) == 0) {
		ok = true;
	}

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_octet");
	} else {
		strcpy(toReturn, svcinfo->data);
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
