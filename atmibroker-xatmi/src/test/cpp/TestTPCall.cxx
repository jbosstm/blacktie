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
#include "XATMITestSuite.h"

#include "xatmi.h"

#include "TestTPCall.h"

extern void test_tpcall_x_octet_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_common_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo);

void TestTPCall::setUp() {
	BaseServerTest::setUp();

	// Do local work
}

void TestTPCall::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPCall::test_tpcall_systemerr() {
	CPPUNIT_ASSERT("Memory fault when name service dies" == "true");

	sendlen = strlen("hello");
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1)) != NULL);
	(void) strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);

	stopNamingService();

	int id = ::tpcall("TestTPCall", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno== TPESYSTEM);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpcall_service") == -1);
}

void TestTPCall::test_tpcall_unknown_service() {
	sendlen = strlen("hello");
	sendbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1);
	rcvbuf = (char *) tpalloc("X_OCTET", NULL, sendlen + 1);
	strcpy(sendbuf, "hello");

	// TODO Changed length from 0 to strlen(sendbuf)+1
	int id = ::tpcall("UNKNOWN_SERVICE", (char *) sendbuf, strlen(sendbuf) + 1, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno== TPENOENT);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(tperrno != 0);
}

void TestTPCall::test_tpcall_x_octet_lessdata() {
	CPPUNIT_ASSERT(strcmp("UNIMPLEMENTED", "TRUE") == 0);
}

void TestTPCall::test_tpcall_x_octet() {
	tpadvertise("test_tpcall_x_octet", test_tpcall_x_octet_service);

	sendlen = strlen("hello");
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc("X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc("X_OCTET", NULL, sendlen)) != NULL);
	(void) strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);

	// TODO Changed length from 0 to sendlen
	int id = ::tpcall("test_tpcall_x_octet", (char *) sendbuf, sendlen, (char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno!= TPEINVAL);
	CPPUNIT_ASSERT(tperrno!= TPENOENT);
	CPPUNIT_ASSERT(tperrno!= TPEITYPE);
	CPPUNIT_ASSERT(tperrno!= TPEOTYPE);
	CPPUNIT_ASSERT(tperrno!= TPETRAN);
	CPPUNIT_ASSERT(tperrno!= TPETIME);
	CPPUNIT_ASSERT(tperrno!= TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno!= TPESVCERR);
	CPPUNIT_ASSERT(tperrno!= TPEBLOCK);
	CPPUNIT_ASSERT(tperrno!= TPGOTSIG);
	CPPUNIT_ASSERT(tperrno!= TPEPROTO);
	CPPUNIT_ASSERT(tperrno!= TPESYSTEM);
	CPPUNIT_ASSERT(tperrno!= TPEOS);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "test_tpcall_x_octet_service") == 0);
}

// 9.1.2
void TestTPCall::test_tpcall_x_common() {
	tpadvertise("test_tpcall_x_common", test_tpcall_x_common_service);

	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc("X_COMMON", "deposit", 0);
	sendbuf = (char*) dptr;
	dptr->acct_no = 12345678;
	dptr->amount = 50;

	int id = ::tpcall("test_tpcall_x_common", (char*) dptr, 0, (char**) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "test_tpcall_x_common_service") == 0);
}
// 9.1.3
void TestTPCall::test_tpcall_x_c_type() {
	tpadvertise("test_tpcall_x_c_type", test_tpcall_x_c_type_service);

	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc("X_C_TYPE", "acct_info", 0);
	sendbuf = (char*) aptr;
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "TOM");
	aptr->balances[0] = 1.1F;
	aptr->balances[1] = 2.2F;

	int id = ::tpcall("test_tpcall_x_c_type", (char*) aptr, 0, (char**) &rcvbuf, &rcvlen, TPNOCHANGE);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "test_tpcall_x_c_type_service") == 0);
}

void test_tpcall_x_octet_service(TPSVCINFO *svcinfo) {
	bool ok = false;
	if (strcmp(svcinfo->data, "hello") == 0) {
		ok = true;
	}

	int len = 60;
	char *toReturn = new char[len];
	if (ok) {
		strcpy(toReturn, "test_tpcall_x_octet_service");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tpcall_x_common_service(TPSVCINFO *svcinfo) {
	bool ok = false;
	DEPOSIT *dptr = (DEPOSIT*) svcinfo->data;
	if (dptr->acct_no == 12345678 && dptr->amount == 50) {
		ok = true;
	}

	int len = 60;
	char *toReturn = new char[len];
	if (ok) {
		strcpy(toReturn, "test_tpcall_x_common_service");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo) {
	bool ok = false;
	ACCT_INFO *aptr = (ACCT_INFO*) svcinfo->data;
	if (aptr->acct_no == 12345678 && strcmp(aptr->name, "TOM") == 0 && aptr->balances[0] == 1.1F && aptr->balances[1] == 2.2F) {
		ok = true;
	}
	int len = 60;
	char *toReturn = new char[len];
	if (ok) {
		strcpy(toReturn, "test_tpcall_x_c_type_service");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
