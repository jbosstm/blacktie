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
#include "TestTPACall.h"
#include "BaseServerTest.h"

#include "xatmi.h"

extern void testtpacall_service(TPSVCINFO *svcinfo);

void TestTPACall::setUp() {
	sendbuf = NULL;
	rcvbuf = NULL;

	// Set up server
	BaseServerTest::setUp();

	// Set up local
	int toCheck = tpadvertise((char*) "TestTPACall", testtpacall_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);
}

void TestTPACall::tearDown() {
	// Clean up local
	if (sendbuf) {
		::tpfree(sendbuf);
	}
	if (rcvbuf) {
		::tpfree(rcvbuf);
	}
	int toCheck = tpunadvertise((char*) "TestTPACall");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPACall::test_tpacall() {
	sendlen = strlen("hello");
	sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen + 1);
	strcpy(sendbuf, "hello");

	int cd = ::tpacall((char*) "TestTPACall", (char *) sendbuf, strlen(sendbuf) + 1, TPNOREPLY);
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(tperrno!= TPEINVAL);
	CPPUNIT_ASSERT(tperrno!= TPENOENT);
	CPPUNIT_ASSERT(tperrno!= TPEITYPE);
	CPPUNIT_ASSERT(tperrno!= TPELIMIT);
	CPPUNIT_ASSERT(tperrno!= TPETRAN);
	CPPUNIT_ASSERT(tperrno!= TPETIME);
	CPPUNIT_ASSERT(tperrno!= TPEBLOCK);
	CPPUNIT_ASSERT(tperrno!= TPGOTSIG);
	CPPUNIT_ASSERT(tperrno!= TPEPROTO);
	CPPUNIT_ASSERT(tperrno!= TPESYSTEM);
	CPPUNIT_ASSERT(tperrno!= TPEOS);
}

void TestTPACall::test_tpacall_systemerr() {
	sendlen = strlen("hello");
	sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen + 1);
	strcpy(sendbuf, "hello");

	int cd = ::tpacall((char*) "TestTPACall", (char *) sendbuf, strlen(sendbuf) + 1, TPNOREPLY);
	CPPUNIT_ASSERT(tperrno== TPESYSTEM);
	CPPUNIT_ASSERT(cd == -1);
}

// 9.1.1
void TestTPACall::test_tpacall_x_octet() {
	char *ptr1, *ptr2;
	sendbuf = tpalloc((char*) "X_OCTET", NULL, 25);
	ptr1 = sendbuf;
	ptr2 = sendbuf + 10;
	strcpy(ptr1, "hello");
	strcpy(ptr2, "goodbye");

	tpacall((char*) "GREETSVC", sendbuf, 25, TPNOREPLY);
}

void testtpacall_service(TPSVCINFO *svcinfo) {
	int len = 20;
	char *toReturn = new char[len];
	strcpy(toReturn, "testtpacall_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
