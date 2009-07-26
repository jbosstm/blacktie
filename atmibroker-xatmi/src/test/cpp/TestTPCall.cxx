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
#include "XATMITestSuite.h"

#include "xatmi.h"

#include "TestTPCall.h"

extern void test_tpcall_x_octet_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_octet_service_zero(TPSVCINFO *svcinfo);
extern void test_tpcall_x_common_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo);

void TestTPCall::setUp() {
	userlogc((char*) "TestTPCall::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;
	BaseServerTest::setUp();
	// Do local work
}

void TestTPCall::tearDown() {
	userlogc((char*) "TestTPCall::tearDown");
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPCall::test_tpcall_systemerr() {
	userlogc((char*) "test_tpcall_systemerr");
	sendlen = strlen("test_tpcall_systemerr") + 1;
	rcvlen = sendlen;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "test_tpcall_systemerr");
	CPPUNIT_ASSERT(tperrno == 0);

	// TODO stopNamingService();

	int id = ::tpcall((char*) "TestTPCall", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == TPESYSTEM);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpcall") == -1);
}

void TestTPCall::test_tpcall_unknown_service() {
	userlogc((char*) "test_tpcall_unknown_service");
	sendlen = strlen("test_tpcall_unknown_service") + 1;
	rcvlen = sendlen;
	sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen);
	rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen);
	strcpy(sendbuf, "test_tpcall_unknown_service");

	int id = ::tpcall((char*) "UNKNOWN_SERVICE", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno == TPENOENT);
	CPPUNIT_ASSERT(id == -1);
	CPPUNIT_ASSERT(tperrno != 0);
}

void TestTPCall::test_tpcall_x_octet_lessdata() {
	userlogc((char*) "test_tpcall_x_octet_lessdata");
	CPPUNIT_FAIL("UNIMPLEMENTED");
}

void TestTPCall::test_tpcall_x_octet() {
	userlogc((char*) "test_tpcall_x_octet");
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service);

	sendlen = strlen("test_tpcall_x_octet") + 1;
	rcvlen = sendlen;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "test_tpcall_x_octet");
	CPPUNIT_ASSERT(tperrno == 0);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno != TPEINVAL);
	CPPUNIT_ASSERT(tperrno != TPENOENT);
	CPPUNIT_ASSERT(tperrno != TPEITYPE);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETRAN);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno != TPGOTSIG);
	CPPUNIT_ASSERT(tperrno != TPEPROTO);
	CPPUNIT_ASSERT(tperrno != TPESYSTEM);
	CPPUNIT_ASSERT(tperrno != TPEOS);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_octet") == 0);
}

void TestTPCall::test_tpcall_x_octet_zero() {
	userlogc((char*) "test_tpcall_x_octet_zero");
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service_zero);

	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, 0))
			!= NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, 0))
			!= NULL);
	CPPUNIT_ASSERT(tperrno == 0);

	sendlen = 0;
	rcvlen = sendlen;

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, 0,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	CPPUNIT_ASSERT(tperrno != TPEINVAL);
	CPPUNIT_ASSERT(tperrno != TPENOENT);
	CPPUNIT_ASSERT(tperrno != TPEITYPE);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETRAN);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno != TPGOTSIG);
	CPPUNIT_ASSERT(tperrno != TPEPROTO);
	CPPUNIT_ASSERT(tperrno != TPESYSTEM);
	CPPUNIT_ASSERT(tperrno != TPEOS);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT(rcvlen == 0);
}

// 9.1.2
void TestTPCall::test_tpcall_x_common() {
	userlogc((char*) "test_tpcall_x_common");
	tpadvertise((char*) "tpcall_x_common", test_tpcall_x_common_service);

	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	rcvlen = 60;

	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	sendbuf = (char*) dptr;
	dptr->acct_no = 12345678;
	dptr->amount = 50;

	int id = ::tpcall((char*) "tpcall_x_common", (char*) dptr, 0,
			(char**) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_common") == 0);
}
// 9.1.3
void TestTPCall::test_tpcall_x_c_type() {
	userlogc((char*) "test_tpcall_x_c_type");
	tpadvertise((char*) "tpcall_x_c_type", test_tpcall_x_c_type_service);

	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	rcvlen = 60;

	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	sendbuf = (char*) aptr;
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "TOM");
	aptr->balances[0] = 1.1F;
	aptr->balances[1] = 2.2F;

	int id = ::tpcall((char*) "tpcall_x_c_type", (char*) aptr, 0,
			(char**) &rcvbuf, &rcvlen, TPNOCHANGE);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id != -1);
	CPPUNIT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_c_type") == 0);
}

void test_tpcall_x_octet_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_octet_service");
	bool ok = false;
	if (svcinfo->data) {
		if (strncmp(svcinfo->data, "test_tpcall_x_octet", svcinfo->len) == 0) {
			ok = true;
		}
	}

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_octet");
	} else {
		strcpy(toReturn, "fail");
		if (svcinfo->data) {
			strcpy(toReturn, svcinfo->data);
		} else {
			strcpy(toReturn, "dud");
		}
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tpcall_x_octet_service_zero(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_octet_service_zero");
	int len = 0;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tpcall_x_common_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_common_service");
	bool ok = false;
	DEPOSIT *dptr = (DEPOSIT*) svcinfo->data;
	if (dptr->acct_no == 12345678 && dptr->amount == 50) {
		ok = true;
	}

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_common");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_c_type_service");
	bool ok = false;
	ACCT_INFO *aptr = (ACCT_INFO*) svcinfo->data;
	if (aptr->acct_no == 12345678 && strcmp(aptr->name, "TOM") == 0
			&& aptr->balances[0] == 1.1F && aptr->balances[1] == 2.2F) {
		ok = true;
	}
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_c_type");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
