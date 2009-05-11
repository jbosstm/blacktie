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

#include "xatmi.h"

#include "TestTPCancel.h"

extern void testtpcancel_service(TPSVCINFO *svcinfo);

void TestTPCancel::setUp() {
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	sendlen = strlen("hello");
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen
			+ 1)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen
			+ 1)) != NULL);
	strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);
	int toCheck = tpadvertise((char*) "TestTPCancel", testtpcancel_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);
}

void TestTPCancel::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPCancel");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPCancel::test_tpcancel() {
	userlogc((char*) "test_tpcancel");
	int cd = ::tpacall((char*) "TestTPCancel", (char *) sendbuf,
			strlen(sendbuf) + 1, 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	// CANCEL THE REQUEST
	int cancelled = ::tpcancel(cd);
	CPPUNIT_ASSERT(cancelled != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(tperrno != TPEBADDESC);
	CPPUNIT_ASSERT(tperrno != TPETRAN);
	CPPUNIT_ASSERT(tperrno != TPEPROTO);
	CPPUNIT_ASSERT(tperrno != TPESYSTEM);
	CPPUNIT_ASSERT(tperrno != TPEOS);

	// FAIL TO RETRIEVE THE RESPONSE
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
	CPPUNIT_ASSERT(valToTest == -1);
}

void TestTPCancel::test_tpcancel_noreply() {
	userlogc((char*) "test_tpcancel_noreply");
	int cd = ::tpacall((char*) "TestTPCancel", (char *) sendbuf,
			strlen(sendbuf) + 1, TPNOREPLY);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	// CANCEL THE REQUEST
	int cancelled = ::tpcancel(cd);
	CPPUNIT_ASSERT(cancelled == -1);
	CPPUNIT_ASSERT(tperrno != 0);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
}

// 8.5
void TestTPCancel::test_tpcancel_baddesc() {
	userlogc((char*) "test_tpcancel_baddesc");
	// CANCEL THE REQUEST
	int cancelled = ::tpcancel(2);
	CPPUNIT_ASSERT(cancelled == -1);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
}

void testtpcancel_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpcancel_service");
	if (!(svcinfo->flags && TPNOREPLY)) {
		int len = 21;
		char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
		strcpy(toReturn, "testtpcancel_service");
		tpreturn(TPSUCCESS, 0, toReturn, len, 0);
	}
}
