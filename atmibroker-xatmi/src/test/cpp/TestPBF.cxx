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
#include "BaseTest.h"

#include "xatmi.h"
#include "malloc.h"

#include "TestPBF.h"

void TestPBF::setUp() {
	userlogc((char*) "TestPBF::setUp");
	m_allocated = NULL;

	BaseTest::setUp();
	// Do local work
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestPBF::tearDown() {
	userlogc((char*) "TestPBF::tearDown");
	if (m_allocated) {
		::tpfree( m_allocated);
		m_allocated = NULL;
	}
	// Do local work
	BaseTest::tearDown();
}

void TestPBF::test_tpalloc() {
	userlogc((char*) "test_tpalloc");
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "R_PBF", (char*) "acct_info", 0);

	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);
	// Won't check length as typtypes does that

	// ASSIGN SOME VALUES
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "12345678901234567890123456789012345678901234567890");
	aptr->balances[0] = 0;
	aptr->balances[1] = 0;

	// CHECK THE ASSIGNATIONS
	CPPUNIT_ASSERT(aptr->acct_no == 12345678);
	CPPUNIT_ASSERT(strcmp(aptr->name,
			"12345678901234567890123456789012345678901234567890") == 0);
	CPPUNIT_ASSERT(aptr->address == NULL || strcmp(aptr->address, "") == 0);
	CPPUNIT_ASSERT(aptr->balances[0] == 0);
	CPPUNIT_ASSERT(aptr->balances[1] == 0);
}

void TestPBF::test_tpalloc_nonzero() {
	userlogc((char*) "test_tpalloc_nonzero");
	m_allocated = tpalloc((char*) "R_PBF", (char*) "acct_info", 10);
	CPPUNIT_ASSERT(m_allocated == NULL);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
}

void TestPBF::test_tpalloc_subtype_required() {
	userlogc((char*) "test_tpalloc_subtype_required");
	m_allocated = tpalloc((char*) "R_PBF", NULL, 0);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
	CPPUNIT_ASSERT(m_allocated == NULL);
}

void TestPBF::test_tpalloc_wrong_subtype() {
	userlogc((char*) "test_tpalloc_subtype_required");
	m_allocated = tpalloc((char*) "R_PBF", (char*) "not_exist", 0);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
	CPPUNIT_ASSERT(m_allocated == NULL);
}

void TestPBF::test_tprealloc() {
	userlogc("test_tprealloc");
	m_allocated = tpalloc((char*) "R_PBF", (char*) "acct_info", 0);
	CPPUNIT_ASSERT(m_allocated != NULL);

	m_allocated = ::tprealloc(m_allocated, 10);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
}

void TestPBF::test_tptypes() {
	userlogc((char*) "test_tptypes_x_common");
	m_allocated = tpalloc((char*) "R_PBF", (char*) "acct_info", 0);
	CPPUNIT_ASSERT(m_allocated != NULL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toTest == 182);
	CPPUNIT_ASSERT(strncmp(type, "R_PBF", 8) == 0);
	CPPUNIT_ASSERT(strcmp(subtype, "acct_info") == 0);
	free(type);
	free(subtype);
}

void TestPBF::test_tpfree() {
	userlogc((char*) "test_tpfree");
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "R_PBF", (char*) "acct_info", 0);
	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);
}
