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

#include "TestTPUnadvertise.h"

extern void testtpunadvertise_service(TPSVCINFO *svcinfo);

void TestTPUnadvertise::setUp() {
	// Setup server
	BaseServerTest::setUp();
	int toCheck = tpadvertise((char*) "TestTPUnadvertise", testtpunadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Do local work
	sendlen = strlen("hello");
	sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1);
	rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1);
	(void) strcpy(sendbuf, "hello");
}

void TestTPUnadvertise::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	tpunadvertise((char*) "TestTPUnadvertise");

	// Clean up server
	BaseServerTest::tearDown();
}

// TODO THIS SHOULD BE ILLEGAL FOR THE CLIENT
void TestTPUnadvertise::test_tpunadvertise() {
	int id = ::tpunadvertise((char*) "TestTPUnadvertise");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(tperrno!= TPEINVAL);
	CPPUNIT_ASSERT(tperrno!= TPENOENT);
	CPPUNIT_ASSERT(tperrno!= TPEPROTO);
	CPPUNIT_ASSERT(tperrno!= TPESYSTEM);
	CPPUNIT_ASSERT(tperrno!= TPEOS);
	CPPUNIT_ASSERT(id != -1);

	id = ::tpcall((char*) "TestTPUnadvertise", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno== TPENOENT);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpunadvertise_service") != 0);
}

void TestTPUnadvertise::test_tpunadvertise_twice() {
	int id = ::tpunadvertise((char*) "TestTPUnadvertise");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(tperrno!= TPEINVAL);
	CPPUNIT_ASSERT(tperrno!= TPENOENT);
	CPPUNIT_ASSERT(tperrno!= TPEPROTO);
	CPPUNIT_ASSERT(tperrno!= TPESYSTEM);
	CPPUNIT_ASSERT(tperrno!= TPEOS);
	CPPUNIT_ASSERT(id != -1);

	id = ::tpunadvertise((char*) "TestTPUnadvertise");
	CPPUNIT_ASSERT(tperrno== TPENOENT);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPUnadvertise::test_tpunadvertise_null() {
	int id = ::tpunadvertise(NULL);
	CPPUNIT_ASSERT(tperrno== TPEINVAL);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPUnadvertise::test_tpunadvertise_empty() {
	int id = ::tpunadvertise((char*) "");
	CPPUNIT_ASSERT(tperrno== TPEINVAL);
	CPPUNIT_ASSERT(id == -1);
}

// 8.4
void TestTPUnadvertise::test_tpunadvertise_not_advertised() {
	int id = ::tpunadvertise((char*) "NONE");
	CPPUNIT_ASSERT(tperrno== TPENOENT);
	CPPUNIT_ASSERT(id == -1);
}

void testtpunadvertise_service(TPSVCINFO *svcinfo) {
	char * toReturn = new char[25];
	strcpy(toReturn, "testtpunadvertise_service");
	// Changed length from 0L to svcinfo->len
	tpreturn(TPSUCCESS, 0, toReturn, 25, 0);
}
