/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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

#include "TestTPAdvertise.h"

extern void testtpadvertise_service(TPSVCINFO *svcinfo);
extern void testtpadvertise_service_2(TPSVCINFO *svcinfo);

void TestTPAdvertise::setUp() {
	// Setup server
	BaseServerTest::setUp();

	// Do local work
	sendlen = strlen("hello");
	sendbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1);
	rcvbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1);
	(void) strcpy(sendbuf, "hello");
}

void TestTPAdvertise::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	::tpunadvertise("TestTPAdvertise");
	::tpunadvertise("1234567890123456");
	::tpunadvertise("1234567890123457");

	// Clean up server
	BaseServerTest::tearDown();
}

// TODO THIS SHOULD BE ILLEGAL FOR THE CLIENT
void TestTPAdvertise::test_tpadvertise_new_service() {
	int id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno!= TPEINVAL);
	CPPUNIT_ASSERT(tperrno!= TPELIMIT);
	CPPUNIT_ASSERT(tperrno!= TPEMATCH);
	CPPUNIT_ASSERT(tperrno!= TPEPROTO);
	CPPUNIT_ASSERT(tperrno!= TPESYSTEM);
	CPPUNIT_ASSERT(tperrno!= TPEOS);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);

	id = ::tpcall("TestTPAdvertise", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpadvertise_service") == 0);
}

void TestTPAdvertise::test_tpadvertise_null_name_null() {
	int id = ::tpadvertise(NULL, testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno== TPEINVAL);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPAdvertise::test_tpadvertise_idempotent() {
	int id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id == 0);
	id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id == 0);
}

void TestTPAdvertise::test_tpadvertise_null_name_empty() {
	int id = ::tpadvertise("", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno== TPEINVAL);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPAdvertise::test_tpadvertise_different_method() {
	int id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id == 0);
	id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service_2);
	CPPUNIT_ASSERT(tperrno== TPEMATCH);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPAdvertise::test_tpadvertise_length_15() {
	int id = ::tpadvertise("12345678901234567", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	id = ::tpadvertise("1234567890123456", testtpadvertise_service_2);
	CPPUNIT_ASSERT(tperrno== TPEMATCH);
	CPPUNIT_ASSERT(id == -1);
}

void TestTPAdvertise::test_tpadvertise_readvertise() {
	int id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	id = ::tpunadvertise("TestTPAdvertise");
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	id = ::tpcall("TestTPAdvertise", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno== TPENOENT);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpadvertise_service") != 0);

	id = ::tpadvertise("TestTPAdvertise", testtpadvertise_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);

	id = ::tpcall("TestTPAdvertise", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpadvertise_service") == 0);
}

void testtpadvertise_service(TPSVCINFO *svcinfo) {
	char * toReturn = new char[23];
	strcpy(toReturn, "testtpadvertise_service");
	// Changed length from 0L to svcinfo->len
	tpreturn(TPSUCCESS, 0, toReturn, 25, 0);
}

void testtpadvertise_service_2(TPSVCINFO *svcinfo) {
	char * toReturn = new char[25];
	strcpy(toReturn, "testtpadvertise_service_2");
	// Changed length from 0L to svcinfo->len
	tpreturn(TPSUCCESS, 0, toReturn, 25, 0);
}
