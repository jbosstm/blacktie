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

#include "TestTPService.h"

extern void testtpservice_service(TPSVCINFO *svcinfo);

void TestTPService::setUp() {
	// Setup server
	BaseServerTest::setUp();
	int toCheck = tpadvertise((char*) "TestTPService", testtpservice_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Do local work
	sendlen = strlen("hello");
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen + 1)) != NULL);
	(void) strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTPService::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	int toCheck = tpunadvertise((char*) "TestTPService");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

#include "malloc.h"
void TestTPService::test_tpservice_notpreturn() {

	int id = ::tpcall((char*) "TestTPService", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);

	int tperrnoToCheck = tperrno;
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	CPPUNIT_ASSERT_MESSAGE(tperrnoS, tperrnoToCheck== TPESVCERR);
	CPPUNIT_ASSERT(id == -1);
}

void testtpservice_service(TPSVCINFO *svcinfo) {
}
