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

#include "xatmi.h"
#include "malloc.h"

#include "TestTPReturn.h"

extern void testtpreturn_service(TPSVCINFO *svcinfo);
extern void testtpreturn_service_tpurcode(TPSVCINFO *svcinfo);

void TestTPReturn::setUp() {
	userlogc((char*) "TestTPReturn::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();
}

void TestTPReturn::tearDown() {
	userlogc((char*) "TestTPReturn::tearDown");
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPReturn");
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

// 8.1 8.3
void TestTPReturn::test_tpreturn_nonservice() {
	int toCheck = tpadvertise((char*) "TestTPReturn", testtpreturn_service);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	userlogc((char*) "test_tpreturn_nonservice");
	// THIS IS ILLEGAL STATE TABLE
	int len = 25;
	char *toReturn = (char*) malloc(len);
	strcpy(toReturn, "test_tpreturn_nonservice");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
	free(toReturn);
}

void TestTPReturn::test_tpreturn_nonbuffer() {
	userlogc((char*) "test_tpreturn_nonbuffer");

	// Do local work
	int toCheck = tpadvertise((char*) "TestTPReturn", testtpreturn_service);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	sendlen = strlen("tprnb") + 1;
	rcvlen = sendlen;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	(void) strcpy(sendbuf, "tprnb");
	BT_ASSERT(tperrno == 0);

	int id = ::tpcall((char*) "TestTPReturn", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, 0);
	long tperrnoS = tperrno;
	BT_ASSERT(id == -1);
	BT_ASSERT(tperrnoS == TPESVCERR);
}

void TestTPReturn::test_tpreturn_tpurcode() {
	userlogc((char*) "test_tpreturn_tpurcode");

	// Do local work
	int toCheck = tpadvertise((char*) "TestTPReturn",
			testtpreturn_service_tpurcode);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	sendlen = 3;
	rcvlen = 1;
	BT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	BT_ASSERT(tperrno == 0);

	strcpy(sendbuf, "24");
	int success = ::tpcall((char*) "TestTPReturn", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(success != -1);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 24);

	strcpy(sendbuf, "77");
	success = ::tpcall((char*) "TestTPReturn", (char *) sendbuf, sendlen,
			(char **) &rcvbuf, &rcvlen, (long) 0);
	BT_ASSERT(success != -1);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(tpurcode == 77);
}

void testtpreturn_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpreturn_service");
	char *toReturn = (char*) malloc(21);
	strcpy(toReturn, "testtpreturn_service");
	tpreturn(TPSUCCESS, 0, toReturn, 21, 0);
	free(toReturn);
}

void testtpreturn_service_tpurcode(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpreturn_service_tpurcode");
	int len = 1;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	if (strncmp(svcinfo->data, "24", 2) == 0) {
		tpreturn(TPSUCCESS, 24, toReturn, len, 0);
	} else {
		tpreturn(TPSUCCESS, 77, toReturn, len, 0);
	}
}
