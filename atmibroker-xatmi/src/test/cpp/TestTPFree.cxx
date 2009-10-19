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
#include "XATMITestSuite.h"

#include "xatmi.h"

#include "TestTPFree.h"

void TestTPFree::setUp() {
	userlogc((char*) "TestTPFree::setUp");
	// Start server
	BaseTest::setUp();
	m_allocated = NULL;

	// Do local work
}

void TestTPFree::tearDown() {
	userlogc((char*) "TestTPFree::tearDown");
	if (m_allocated) {
		// Do local work
		::tpfree( m_allocated);
	}

	// Clean up server
	BaseTest::tearDown();
}

// X_OCTET
void TestTPFree::test_tpfree_alloc_x_octet() {
	userlogc((char*) "test_tpfree_alloc_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	CPPUNIT_ASSERT(m_allocated != NULL);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 int toTest = ::tptypes(m_allocated, type, NULL);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_OCTET", 8) != 0);
	 */
}

void TestTPFree::test_tpfree_realloc_x_octet() {
	userlogc((char*) "test_tpfree_realloc_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	CPPUNIT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 20);
	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 int toTest = ::tptypes(m_allocated, type, NULL);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_OCTET", 8) != 0);
	 */
}

void TestTPFree::test_tpfree_free_free_x_octet() {
	userlogc((char*) "test_tpfree_free_free_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	CPPUNIT_ASSERT(m_allocated != NULL);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 int toTest = ::tptypes(m_allocated, type, NULL);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_OCTET", 8) != 0);

	 ::tpfree(m_allocated);
	 CPPUNIT_ASSERT(tperrno == 0);
	 */
}

// 8.2
void TestTPFree::test_tpfree_nonbuffer() {
	userlogc((char*) "test_tpfree_nonbuffer");
	char* unallocated = (char*) "nonbuffer";
	::tpfree(unallocated);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(strcmp(unallocated, "nonbuffer") == 0);
}

// X_COMMON
void TestTPFree::test_tpfree_alloc_x_common() {
	userlogc((char*) "test_tpfree_alloc_x_common");
	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	m_allocated = (char*) dptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_COMMON", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "deposit") != 0);
	 */
}

void TestTPFree::test_tpfree_realloc_x_common() {
	userlogc((char*) "test_tpfree_realloc_x_common");
	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	m_allocated = (char*) dptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tprealloc(m_allocated, 2048);
	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_COMMON", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "deposit") != 0);
	 */
}

void TestTPFree::test_tpfree_free_free_x_common() {
	userlogc((char*) "test_tpfree_free_free_x_common");
	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	m_allocated = (char*) dptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*
	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_COMMON", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "deposit") != 0);

	 ::tpfree(m_allocated);
	 CPPUNIT_ASSERT(tperrno == 0);
	 */
}

// X_C_TYPE
void TestTPFree::test_tpfree_alloc_x_c_type() {
	userlogc((char*) "test_tpfree_alloc_x_c_type");
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*

	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_C_TYPE", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "acct_info") != 0);
	 */
}

void TestTPFree::test_tpfree_realloc_x_c_type() {
	userlogc((char*) "test_tpfree_realloc_x_c_type");
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tprealloc(m_allocated, 20);
	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*

	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_C_TYPE", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "acct_info") != 0);
	 */
}

void TestTPFree::test_tpfree_free_free_x_c_type() {
	userlogc((char*) "test_tpfree_free_free_x_c_type");
	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	m_allocated = (char*) aptr;
	CPPUNIT_ASSERT(m_allocated != NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	::tpfree( m_allocated);
	m_allocated = NULL;
	CPPUNIT_ASSERT(tperrno == 0);

	// Once tpfree returns, ptr should not be passed as an argument to any XATMI routine
	/*

	 char type[8];
	 char subtype[16];
	 int toTest = ::tptypes(m_allocated, type, subtype);
	 CPPUNIT_ASSERT(tperrno== TPEINVAL);
	 CPPUNIT_ASSERT(toTest == -1);
	 CPPUNIT_ASSERT(strncmp(type, "X_C_TYPE", 8) != 0);
	 CPPUNIT_ASSERT(strcmp(subtype, "acct_info") != 0);

	 ::tpfree(m_allocated);
	 CPPUNIT_ASSERT(tperrno == 0);
	 */
}
