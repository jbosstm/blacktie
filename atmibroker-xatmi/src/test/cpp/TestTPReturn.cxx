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

#include "TestTPReturn.h"

extern void testtpreturn_service(TPSVCINFO *svcinfo);

void TestTPReturn::setUp() {
	userlogc((char*) "TestTPReturn::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	int toCheck = tpadvertise((char*) "TestTPReturn", testtpreturn_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);
}

void TestTPReturn::tearDown() {
	userlogc((char*) "TestTPReturn::tearDown");
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPReturn");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

// 8.1 8.3
void TestTPReturn::test_tpreturn_nonservice() {
	userlogc((char*) "test_tpreturn_nonservice");
	// THIS IS ILLEGAL STATE TABLE
	int len = 25;
	char *toReturn = new char[len];
	strcpy(toReturn, "test_tpreturn_nonservice");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void TestTPReturn::test_tpreturn_nonbuffer() {
	userlogc((char*) "test_tpreturn_nonbuffer");
	sendlen = strlen("hello");
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1)) != NULL);
	(void) strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);

	int id = ::tpcall((char*) "TestTPReturn", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, 0);
	long tperrnoS = tperrno;
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(tperrnoS == TPESVCERR);
}

void testtpreturn_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpreturn_service");
	char *toReturn = new char[21];
	strcpy(toReturn, "testtpreturn_service");
	tpreturn(TPSUCCESS, 0, toReturn, 21, 0);
}
