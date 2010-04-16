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

#include "BaseServerTest.h"
#include "XATMITestSuite.h"

#include "xatmi.h"

#include "TestTPCall.h"
#include "Sleeper.h"

extern void test_tpcall_x_octet_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_octet_service_zero(TPSVCINFO *svcinfo);
extern void test_tpcall_x_common_service(TPSVCINFO *svcinfo);
extern void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo);

extern void test_tpcall_TPNOBLOCK(TPSVCINFO *svcinfo);
extern void test_tpcall_TPNOTIME(TPSVCINFO *svcinfo);

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
	::tpfree( sendbuf);
	::tpfree( rcvbuf);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPCall::test_tpcall_systemerr() {
	userlogc((char*) "test_tpcall_systemerr");
	sendlen = strlen("test_tpcall_systemerr") + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "test_tpcall_systemerr");
	BT_ASSERT(tperrno == 0);

	// TODO stopNamingService();

	int id = ::tpcall((char*) "TestTPCall", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(tperrno == TPESYSTEM);
	BT_ASSERT(id == -1);
	BT_ASSERT(strcmp(rcvbuf, "testtpcall") == -1);
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
	BT_ASSERT(tperrno == TPENOENT);
	BT_ASSERT(id == -1);
	BT_ASSERT(tperrno != 0);
}

void TestTPCall::test_tpcall_x_octet_lessdata() {
	userlogc((char*) "test_tpcall_x_octet_lessdata");
	BT_FAIL("UNIMPLEMENTED");
}

void TestTPCall::test_tpcall_null_service() {
	userlogc((char*) "test_tpcall_x_octet");
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service);

	sendlen = strlen("test_tpcall_x_octet") + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "test_tpcall_x_octet");
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	int id = ::tpcall(NULL, (char *) sendbuf, sendlen, (char **) &rcvbuf,
			&rcvlen, (long) 0);
	BT_ASSERT(tperrno == TPEINVAL);
	BT_ASSERT(id == -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_octet") != 0);
}

void TestTPCall::test_tpcall_x_octet() {
	userlogc((char*) "test_tpcall_x_octet");
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service);

	sendlen = strlen("test_tpcall_x_octet") + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "test_tpcall_x_octet");
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(tperrno != TPEINVAL);
	BT_ASSERT(tperrno != TPENOENT);
	BT_ASSERT(tperrno != TPEITYPE);
	BT_ASSERT(tperrno != TPEOTYPE);
	BT_ASSERT(tperrno != TPETRAN);
	BT_ASSERT(tperrno != TPETIME);
	BT_ASSERT(tperrno != TPESVCFAIL);
	BT_ASSERT(tperrno != TPESVCERR);
	BT_ASSERT(tperrno != TPEBLOCK);
	BT_ASSERT(tperrno != TPGOTSIG);
	BT_ASSERT(tperrno != TPEPROTO);
	BT_ASSERT(tperrno != TPESYSTEM);
	BT_ASSERT(tperrno != TPEOS);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 20);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_octet") == 0);
}

void TestTPCall::test_tpcall_x_octet_zero() {
	userlogc((char*) "test_tpcall_x_octet_zero");
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_x_octet_service_zero);

	BT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, 0))
			!= NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, 0))
			!= NULL);
	BT_ASSERT(tperrno == 0);

	sendlen = 0;
	rcvlen = sendlen;

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, 0,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(tperrno != TPEINVAL);
	BT_ASSERT(tperrno != TPENOENT);
	BT_ASSERT(tperrno != TPEITYPE);
	BT_ASSERT(tperrno != TPEOTYPE);
	BT_ASSERT(tperrno != TPETRAN);
	BT_ASSERT(tperrno != TPETIME);
	BT_ASSERT(tperrno != TPESVCFAIL);
	BT_ASSERT(tperrno != TPESVCERR);
	BT_ASSERT(tperrno != TPEBLOCK);
	BT_ASSERT(tperrno != TPGOTSIG);
	BT_ASSERT(tperrno != TPEPROTO);
	BT_ASSERT(tperrno != TPESYSTEM);
	BT_ASSERT(tperrno != TPEOS);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 21);
	BT_ASSERT(id != -1);
	BT_ASSERT(rcvlen == 0);
}

#include "malloc.h"
// 9.1.2
void TestTPCall::test_tpcall_x_common() {
	userlogc((char*) "test_tpcall_x_common");
	tpadvertise((char*) "tpcall_x_common", test_tpcall_x_common_service);

	DEPOSIT *dptr;
	dptr = (DEPOSIT*) tpalloc((char*) "X_COMMON", (char*) "deposit", 0);
	rcvlen = 60;

	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	sendbuf = (char*) dptr;
	dptr->acct_no = 12345678;
	dptr->amount = 50;

	//	userlogc("%d %d %d", sizeof(long), sizeof(int), sizeof(short));
	//	char foo[sizeof(short)]; // 8
	//	memcpy(foo, &sendbuf[8], sizeof(short));
	//	short* bar = (short*) foo;
	//	short barbar = *bar;

	int id = ::tpcall((char*) "tpcall_x_common", (char*) dptr, 0,
			(char**) &rcvbuf, &rcvlen, 0);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 22);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_common") == 0);
}
// 9.1.3
void TestTPCall::test_tpcall_x_c_type() {
	userlogc((char*) "test_tpcall_x_c_type");
	tpadvertise((char*) "tpcall_x_c_type", test_tpcall_x_c_type_service);

	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	rcvlen = 60;

	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	sendbuf = (char*) aptr;
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "TOM");
	aptr->foo[0] = 1.1F;
	aptr->foo[1] = 2.2F;

	aptr->balances[0] = 1.1;
	aptr->balances[1] = 2.2;

	int id = ::tpcall((char*) "tpcall_x_c_type", (char*) aptr, 0,
			(char**) &rcvbuf, &rcvlen, TPNOCHANGE);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 23);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_c_type") == 0);
}

void TestTPCall::test_tpcall_with_TPNOCHANGE() {
	userlogc((char*) "test_tpcall_with_TPNOCHANGE");
	tpadvertise((char*) "tpcall_x_c_type", test_tpcall_x_c_type_service);

	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	rcvlen = 60;

	rcvbuf = (char *) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(rcvbuf != NULL);
	sendbuf = (char*) aptr;
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "TOM");
	aptr->foo[0] = 1.1F;
	aptr->foo[1] = 2.2F;

	aptr->balances[0] = 1.1;
	aptr->balances[1] = 2.2;

	int id = ::tpcall((char*) "tpcall_x_c_type", (char*) aptr, 0,
			(char**) &rcvbuf, &rcvlen, TPNOCHANGE);
	BT_ASSERT(tperrno == TPEOTYPE);
	BT_ASSERT(id == -1);

	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	long toTest = ::tptypes(rcvbuf, type, subtype);
	BT_ASSERT(strncmp(type, "X_C_TYPE", 8) == 0);
	BT_ASSERT(strncmp(subtype, "acct_info", 16) == 0);

	char* toTestS = (char*) malloc(110);
	sprintf(toTestS, "%d", toTest);
	BT_ASSERT_MESSAGE(toTestS, toTest == sizeof(ACCT_INFO));
	free(toTestS);

	free(type);
	free(subtype);

}

void TestTPCall::test_tpcall_without_TPNOCHANGE() {
	userlogc((char*) "test_tpcall_without_TPNOCHANGE");
	tpadvertise((char*) "tpcall_x_c_type", test_tpcall_x_c_type_service);

	ACCT_INFO *aptr;
	aptr = (ACCT_INFO*) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	rcvlen = 60;

	rcvbuf = (char *) tpalloc((char*) "X_C_TYPE", (char*) "acct_info", 0);
	BT_ASSERT(rcvbuf != NULL);
	sendbuf = (char*) aptr;
	aptr->acct_no = 12345678;
	strcpy(aptr->name, "TOM");
	aptr->foo[0] = 1.1F;
	aptr->foo[1] = 2.2F;

	aptr->balances[0] = 1.1;
	aptr->balances[1] = 2.2;

	int id = ::tpcall((char*) "tpcall_x_c_type", (char*) aptr, 0,
			(char**) &rcvbuf, &rcvlen, 0);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 23);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, "tpcall_x_c_type") == 0);
	BT_ASSERT(rcvlen == 60);
	char* type = (char*) malloc(8);
	char* subtype = (char*) malloc(16);
	long toTest = ::tptypes(rcvbuf, type, subtype);
	BT_ASSERT(strcmp(type, "X_OCTET") == 0);
	BT_ASSERT(strcmp(subtype, "") == 0);

	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	free(type);
	free(subtype);
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
	tpreturn(TPSUCCESS, 20, toReturn, len, 0);
}

void test_tpcall_x_octet_service_zero(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_octet_service_zero");
	int len = 0;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	tpreturn(TPSUCCESS, 21, toReturn, len, 0);
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
	tpreturn(TPSUCCESS, 22, toReturn, len, 0);
}

void test_tpcall_x_c_type_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_x_c_type_service");
	bool ok = false;
	ACCT_INFO *aptr = (ACCT_INFO*) svcinfo->data;
	bool acctEq = aptr->acct_no == 12345678;
	bool nameEq = strcmp(aptr->name, "TOM") == 0;
	bool fooEq = aptr->foo[0] == 1.1F && aptr->foo[1] == 2.2F;
	bool balsEq = aptr->balances[0] == 1.1 && aptr->balances[1] == 2.2;
	if (acctEq && nameEq && fooEq && balsEq) {
		ok = true;
	}
	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (ok) {
		strcpy(toReturn, "tpcall_x_c_type");
	} else {
		strcpy(toReturn, "fail");
	}
	tpreturn(TPSUCCESS, 23, toReturn, len, 0);
}

void TestTPCall::test_tpcall_with_TPNOBLOCK() {
	char* toTest = (char*) "test_tpcall_TPNOBLOCK";
	userlogc(toTest);
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_TPNOBLOCK);

	sendlen = strlen(toTest) + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, toTest);
	BT_ASSERT(tperrno == 0);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, TPNOBLOCK);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(id != -1);
	BT_ASSERT(strcmp(rcvbuf, toTest) == 0);
}

void TestTPCall::test_tpcall_without_TPNOBLOCK() {
	char* toTest = (char*) "test_tpcall_no_TPNOBLOCK";
	userlogc(toTest);
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_TPNOBLOCK);

	sendlen = strlen(toTest) + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, toTest);
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, toTest) == 0);
}

void TestTPCall::test_tpcall_without_TPNOTIME() {
	char* toTest = (char*) "test_tpcall_no_TPNOTIME";
	userlogc(toTest);
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_TPNOTIME);

	sendlen = strlen(toTest) + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, toTest);
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, 0);
	BT_ASSERT(tperrno == TPETIME);
	BT_ASSERT(id == -1);
	BT_ASSERT(strcmp(rcvbuf, toTest) == -1);
}

void TestTPCall::test_tpcall_with_TPNOTIME() {
	char* toTest = (char*) "test_tpcall_TPNOTIME";
	userlogc(toTest);
	tpadvertise((char*) "tpcall_x_octet", test_tpcall_TPNOTIME);

	sendlen = strlen(toTest) + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, toTest);
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);

	int id = ::tpcall((char*) "tpcall_x_octet", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, TPNOTIME);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(id != -1);
	BT_ASSERT_MESSAGE(rcvbuf, strcmp(rcvbuf, toTest) == 0);
}

void test_tpcall_TPNOBLOCK(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_TPNOBLOCK");
	::sleeper(5);
	tpreturn(TPSUCCESS, 0, svcinfo->data, svcinfo->len, 0);
}

void test_tpcall_TPNOTIME(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_tpcall_TPNOTIME");
	::sleeper(31);
	tpreturn(TPSUCCESS, 0, svcinfo->data, svcinfo->len, 0);
}
