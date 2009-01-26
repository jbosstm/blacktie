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
#include "XATMITestSuite.h"
#include "BaseServerTest.h"

#include "xatmi.h"

#include "TestTPFreeService.h"

extern void testtpfreeservice_service(TPSVCINFO *svcinfo);

void TestTPFreeService::setUp() {
	// Start server
	BaseServerTest::setUp();

	// Do local work
	int toCheck = tpadvertise((char*) "TestTPFree", testtpfreeservice_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);
}

void TestTPFreeService::tearDown() {
	// Do local work
	::tpfree(m_allocated);
	::tpfree(m_rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPFree");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPFreeService::test_tpfree_x_octet() {
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	CPPUNIT_ASSERT(m_allocated != NULL);

	int toCheck = ::tpcall((char*) "TestTPFree", (char*) m_allocated, 0, (char**) &m_rcvbuf, &m_rcvlen, 0);
	CPPUNIT_ASSERT(toCheck != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(m_rcvbuf[0] == '0');
}

void TestTPFreeService::test_tpfree_x_common() {
	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	m_allocated = (char*) dptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	int toCheck = ::tpcall((char*) "TestTPFree", (char*) m_allocated, 0, (char**) &m_rcvbuf, &m_rcvlen, 0);
	CPPUNIT_ASSERT(toCheck != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(m_rcvbuf[0] == '0');
}

void TestTPFreeService::test_tpfree_x_c_type() {
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	int toCheck = ::tpcall((char*) "TestTPFree", (char*) m_allocated, 0, (char**) &m_rcvbuf, &m_rcvlen, 0);
	CPPUNIT_ASSERT(toCheck != -1);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(m_rcvbuf[0] == '0');
}

void testtpfreeservice_service(TPSVCINFO *svcinfo) {
	// Allocate a buffer to return
	char *toReturn = tpalloc((char*) "X_OCTET", (char*) "acct_info", 1);

	// Free should be idempotent on the inbound buffer
	::tpfree(svcinfo->data);

	// Get the data from tptypes still
	char type[8];
	char subtype[16];
	int toTest = ::tptypes(svcinfo->data, type, subtype);

	// Check the values of tptypes (should still have been valid
	if (toTest == -1 || tperrno == TPEINVAL) {
		// False
		toReturn[0] = '0';
	} else {
		// True
		toReturn[0] = '1';
	}

	// Return the data
	tpreturn(TPSUCCESS, 0, toReturn, 1, 0);
}
