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
#include "TestTPACall.h"
#include "BaseServerTest.h"

#include "xatmi.h"
#include "malloc.h"

#if defined(__cplusplus)
extern "C" {
#endif
extern void testtpacall_service(TPSVCINFO *svcinfo);
#if defined(__cplusplus)
}
#endif

void TestTPACall::setUp() {
	userlogc((char*) "TestTPACall::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Set up server
	BaseServerTest::setUp();

	// Set up local
	int toCheck = tpadvertise((char*) "TestTPACall", testtpacall_service);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);
}

void TestTPACall::tearDown() {
	userlogc((char*) "TestTPACall::tearDown");
	// Clean up local
	if (sendbuf) {
		::tpfree(sendbuf);
	}
	if (rcvbuf) {
		::tpfree(rcvbuf);
	}
	int toCheck = tpunadvertise((char*) "TestTPACall");
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPACall::test_tpacall() {
	userlogc((char*) "test_tpacall");

	sendlen = strlen("test_tpacall") + 1;
	sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, "test_tpacall");
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	BT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	free(tperrnoS);
	int cd = ::tpacall((char*) "TestTPACall", (char *) sendbuf, sendlen,
			TPNOREPLY);

	BT_ASSERT(tperrno != TPEINVAL);
	BT_ASSERT(tperrno != TPENOENT);
	BT_ASSERT(tperrno != TPEITYPE);
	BT_ASSERT(tperrno != TPELIMIT);
	BT_ASSERT(tperrno != TPETRAN);
	BT_ASSERT(tperrno != TPETIME);
	BT_ASSERT(tperrno != TPEBLOCK);
	BT_ASSERT(tperrno != TPGOTSIG);
	BT_ASSERT(tperrno != TPEPROTO);
	BT_ASSERT(tperrno != TPESYSTEM);
	BT_ASSERT(tperrno != TPEOS);
	BT_ASSERT(tperrno == 0);

	char* cdS = (char*) malloc(110);
	sprintf(cdS, "%d", cd);
	BT_ASSERT_MESSAGE(cdS, cd == 0);
	free(cdS);
}

void TestTPACall::test_tpacall_systemerr() {
	userlogc((char*) "test_tpacall_systemerr");
	sendlen = strlen("test_tpacall_systemerr") + 1;
	sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, "test_tpacall_systemerr");

	int cd = ::tpacall((char*) "TestTPACall", (char *) sendbuf, sendlen,
			TPNOREPLY);
	BT_ASSERT(tperrno == TPESYSTEM);
	BT_ASSERT(cd == -1);
}

// 9.1.1
void TestTPACall::test_tpacall_x_octet() {
	userlogc((char*) "test_tpacall_x_octet");
	char *ptr1, *ptr2;
	sendbuf = tpalloc((char*) "X_OCTET", NULL, 25);
	ptr1 = sendbuf;
	ptr2 = sendbuf + 10;
	strcpy(ptr1, "hello");
	strcpy(ptr2, "goodbye");

	int cd = tpacall((char*) "GREETSVC", sendbuf, 25, TPNOREPLY);
	BT_ASSERT(tperrno == TPENOENT);
	char* cdS = (char*) malloc(110);
	sprintf(cdS, "%d", cd);
	BT_ASSERT_MESSAGE(cdS, cd == -1);
	free(cdS);
}

void testtpacall_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpacall_service");
	int len = 20;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "testtpacall_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
