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
#include "TestAssert.h"
#include "BaseTest.h"

#include "xatmi.h"
#include "XATMITestSuite.h"

#include "TestTPRealloc.h"
#include "malloc.h"

void TestTPRealloc::setUp() {
	userlogc((char*) "TestTPRealloc::setUp");
	m_allocated = NULL;
	m_nonallocated = NULL;
	BaseTest::setUp();

	// Do local work
}

void TestTPRealloc::tearDown() {
	userlogc((char*) "TestTPRealloc::tearDown");
	if (m_allocated) {
		// Do local work
		::tpfree( m_allocated);
		m_allocated = NULL;
	}
	if (m_nonallocated != NULL) {

		free( m_nonallocated);
		m_nonallocated = NULL;
	}
	BaseTest::tearDown();
}

// X_OCTET
void TestTPRealloc::test_tprealloc_negative_x_octet() {
	userlogc("test_tprealloc_negative_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, -1);
	BT_ASSERT(tperrno == TPEINVAL);
}

// THIS DOES NOT WORK AS YOU CANNOT REALLOC A ZERO BUFFER AS IT CANT BE FOUND
void TestTPRealloc::test_tprealloc_zero_x_octet() {
	userlogc("test_tprealloc_zero_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 0);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_OCTET", 8) == 0);
	BT_ASSERT(strcmp(subtype, "") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == 10);
}

void TestTPRealloc::test_tprealloc_larger_x_octet() {
	userlogc("test_tprealloc_larger_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	m_allocated = ::tprealloc(m_allocated, 20);
	BT_ASSERT(tperrno == 0);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_OCTET", 8) == 0);
	BT_ASSERT(strcmp(subtype, "") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == 20);
}

void TestTPRealloc::test_tprealloc_smaller_x_octet() {
	userlogc("test_tprealloc_smaller_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	m_allocated = ::tprealloc(m_allocated, 5);
	BT_ASSERT(tperrno == 0);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_OCTET", 8) == 0);
	BT_ASSERT(strcmp(subtype, "") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == 5);
}

void TestTPRealloc::test_tprealloc_samesize_x_octet() {
	userlogc("test_tprealloc_samesize_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	m_allocated = ::tprealloc(m_allocated, 10);
	BT_ASSERT(tperrno == 0);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_OCTET", 8) == 0);
	BT_ASSERT(strcmp(subtype, "") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == 10);
}

void TestTPRealloc::test_tprealloc_multi_x_octet() {
	userlogc("test_tprealloc_multi_x_octet");
	m_allocated = tpalloc((char*) "X_OCTET", NULL, 10);
	BT_ASSERT(m_allocated != NULL);

	for (int i = 32; i <= 128; i++) {
		m_allocated = ::tprealloc(m_allocated, i);
		BT_ASSERT(tperrno == 0);

		char* type = (char*) malloc(8);
		char* subtype = (char*) malloc(16);
		int toTest = ::tptypes(m_allocated, type, subtype);
		BT_ASSERT(strncmp(type, "X_OCTET", 8) == 0);
		BT_ASSERT(strcmp(subtype, "") == 0);
		free(type);
		free(subtype);
		BT_ASSERT(tperrno == 0);
		BT_ASSERT(toTest == i);
	}
}

// 8.2
void TestTPRealloc::test_tprealloc_nonbuffer() {
	userlogc("test_tprealloc_nonbuffer");
	m_nonallocated = (char*) malloc(10);
	char* toFree = m_nonallocated;
	m_nonallocated = ::tprealloc(m_nonallocated, 10);
	BT_ASSERT(tperrno == TPEINVAL);
	free(toFree);
}

void TestTPRealloc::test_tprealloc_null() {
	userlogc("test_tprealloc_null");
	m_nonallocated = ::tprealloc(NULL, 10);
	BT_ASSERT(tperrno == TPEINVAL);
}

// X_COMMON
void TestTPRealloc::test_tprealloc_negative_x_common() {
	userlogc("test_tprealloc_negative_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, -1);
	BT_ASSERT(tperrno == TPEINVAL);
}

void TestTPRealloc::test_tprealloc_zero_x_common() {
	userlogc("test_tprealloc_zero_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 0);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_COMMON", 8) == 0);
	BT_ASSERT(strcmp(subtype, "deposit") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(DEPOSIT));
}

void TestTPRealloc::test_tprealloc_larger_x_common() {
	userlogc("test_tprealloc_larger_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 3027);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_COMMON", 8) == 0);
	BT_ASSERT(strcmp(subtype, "deposit") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(DEPOSIT));
}

void TestTPRealloc::test_tprealloc_smaller_x_common() {
	userlogc("test_tprealloc_smaller_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 512);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_COMMON", 8) == 0);
	BT_ASSERT(strcmp(subtype, "deposit") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(DEPOSIT));
}

void TestTPRealloc::test_tprealloc_samesize_x_common() {
	userlogc("test_tprealloc_samesize_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 2048);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_COMMON", 8) == 0);
	BT_ASSERT(strcmp(subtype, "deposit") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(DEPOSIT));
}

void TestTPRealloc::test_tprealloc_multi_x_common() {
	userlogc("test_tprealloc_multi_x_common");
	m_allocated = tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	BT_ASSERT(m_allocated != NULL);

	for (int i = 1024; i <= 1124; i++) {
		::tprealloc(m_allocated, i);
		BT_ASSERT(tperrno == TPEINVAL);

		char* type = (char*) malloc(8);
		char* subtype = (char*) malloc(16);
		int toTest = ::tptypes(m_allocated, type, subtype);
		BT_ASSERT(strncmp(type, "X_COMMON", 8) == 0);
		BT_ASSERT(strcmp(subtype, "deposit") == 0);
		free(type);
		free(subtype);
		BT_ASSERT(tperrno == 0);
		BT_ASSERT(toTest == sizeof(DEPOSIT));
	}
}

// X_C_TYPE
void TestTPRealloc::test_tprealloc_negative_x_c_type() {
	userlogc("test_tprealloc_negative_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, -1);
	BT_ASSERT(tperrno == TPEINVAL);
}

void TestTPRealloc::test_tprealloc_zero_x_c_type() {
	userlogc("test_tprealloc_zero_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 0);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
	BT_ASSERT(strcmp(subtype, "acct_info") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(ACCT_INFO));
}

void TestTPRealloc::test_tprealloc_larger_x_c_type() {
	userlogc("test_tprealloc_larger_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 3072);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
	BT_ASSERT(strcmp(subtype, "acct_info") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(ACCT_INFO));
}

void TestTPRealloc::test_tprealloc_smaller_x_c_type() {
	userlogc("test_tprealloc_smaller_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 512);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
	BT_ASSERT(strcmp(subtype, "acct_info") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(ACCT_INFO));
}

void TestTPRealloc::test_tprealloc_samesize_x_c_type() {
	userlogc("test_tprealloc_samesize_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	::tprealloc(m_allocated, 2048);
	BT_ASSERT(tperrno == TPEINVAL);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	int toTest = ::tptypes(m_allocated, type, subtype);
	BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
	BT_ASSERT(strcmp(subtype, "acct_info") == 0);
	free(type);
	free(subtype);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toTest == sizeof(ACCT_INFO));
}

void TestTPRealloc::test_tprealloc_multi_x_c_type() {
	userlogc("test_tprealloc_multi_x_c_type");
	m_allocated = tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(m_allocated != NULL);

	for (int i = 1024; i <= 1124; i++) {
		::tprealloc(m_allocated, i);
		BT_ASSERT(tperrno == TPEINVAL);

		char* type = (char*) malloc(8);
		char* subtype = (char*) malloc(16);
		int toTest = ::tptypes(m_allocated, type, subtype);
		BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
		BT_ASSERT(strcmp(subtype, "acct_info") == 0);
		free(type);
		free(subtype);
		BT_ASSERT(tperrno == 0);
		BT_ASSERT(toTest == sizeof(ACCT_INFO));
	}
}
