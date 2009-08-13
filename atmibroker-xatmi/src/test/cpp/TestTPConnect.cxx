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

#include "TestTPConnect.h"

extern void testtpconnect_service(TPSVCINFO *svcinfo);

void TestTPConnect::setUp() {
	userlogc((char*) "TestTPConnect::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	cd = -1;
	cd2 = -1;
	int toCheck = tpadvertise((char*) "TestTPConnect", testtpconnect_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	sendlen = strlen("connect") + 1;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT(
			(rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen))
					!= NULL);
	strcpy(sendbuf, "connect");
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTPConnect::tearDown() {
	userlogc((char*) "TestTPConnect::tearDown");
	// Do local work
	if (cd != -1) {
		::tpdiscon(cd);
	}
	if (cd2 != -1) {
		::tpdiscon(cd2);
	}
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPConnect");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPConnect::test_tpconnect() {
	userlogc((char*) "test_tpconnect");
	cd = ::tpconnect((char*) "TestTPConnect", sendbuf, sendlen, TPRECVONLY);
	CPPUNIT_ASSERT(cd != -1);
}

void TestTPConnect::test_tpconnect_double_connect() {
	userlogc((char*) "test_tpconnect_double_connect");
	cd = ::tpconnect((char*) "TestTPConnect", sendbuf, sendlen, TPRECVONLY);
	cd2 = ::tpconnect((char*) "TestTPConnect", sendbuf, sendlen, TPRECVONLY);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(cd2 != -1);
	CPPUNIT_ASSERT(cd != cd2);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(tperrno != TPEINVAL);
	CPPUNIT_ASSERT(tperrno != TPENOENT);
	CPPUNIT_ASSERT(tperrno != TPEITYPE);
	CPPUNIT_ASSERT(tperrno != TPELIMIT);
	CPPUNIT_ASSERT(tperrno != TPETRAN);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno != TPGOTSIG);
	CPPUNIT_ASSERT(tperrno != TPEPROTO);
	CPPUNIT_ASSERT(tperrno != TPESYSTEM);
	CPPUNIT_ASSERT(tperrno != TPEOS);
}

void TestTPConnect::test_tpconnect_nodata() {
	userlogc((char*) "test_tpconnect_nodata");
	cd = ::tpconnect((char*) "TestTPConnect", NULL, 0, TPRECVONLY);
	CPPUNIT_ASSERT(cd != -1);
}

void testtpconnect_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpconnect_service");
	tpreturn(TPSUCCESS, 0, NULL, 0, 0);
}
