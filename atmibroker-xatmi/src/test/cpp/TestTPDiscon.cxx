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
#include "Sleeper.h"

#include "xatmi.h"

#include "TestTPDiscon.h"

extern void testtpdiscon_service(TPSVCINFO *svcinfo);

void TestTPDiscon::setUp() {
	userlogc((char*) "TestTPDiscon::setUp");
	sendbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	sendlen = strlen("discon") + 1;
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	strcpy(sendbuf, "discon");
	CPPUNIT_ASSERT(tperrno == 0);

	int toCheck = tpadvertise((char*) "TestTPDiscon", testtpdiscon_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	cd = ::tpconnect((char*) "TestTPDiscon", sendbuf, sendlen, TPSENDONLY);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(cd != -1);
}

void TestTPDiscon::tearDown() {
	userlogc((char*) "TestTPDiscon::tearDown");
	// Do local work
	::tpfree(sendbuf);
	if (cd != -1) {
		::tpdiscon(cd);
	}
	int toCheck = tpunadvertise((char*) "TestTPDiscon");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPDiscon::test_tpdiscon() {
	userlogc((char*) "test_tpdiscon");
	::tpdiscon(cd);
	CPPUNIT_ASSERT(tperrno == 0);
	cd = -1;
}

void TestTPDiscon::test_tpdiscon_baddescr() {
	userlogc((char*) "test_tpdiscon_baddescr");
	::tpdiscon(cd + 1);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
}

void TestTPDiscon::test_tpdiscon_negdescr() {
	userlogc((char*) "test_tpdiscon_negdescr");
	::tpdiscon(-1);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
}

void testtpdiscon_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpdiscon_service");
	::sleeper(2);
}
