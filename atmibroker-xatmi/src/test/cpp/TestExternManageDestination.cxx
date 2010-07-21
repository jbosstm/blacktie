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
#include "TestExternManageDestination.h"
#include "BaseServerTest.h"

#include "xatmi.h"
#include "malloc.h"

#if defined(__cplusplus)
extern "C" {
#endif
extern void test_extern_service(TPSVCINFO *svcinfo);
#if defined(__cplusplus)
}
#endif

void TestExternManageDestination::setUp() {
	userlogc((char*) "TestExternManageDestination::setUp");
	unadvertised = false;

	// Set up server
	BaseServerTest::setUp();

	int toCheck = tpadvertise((char*) "TestOne", test_extern_service);
	userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);
}

void TestExternManageDestination::tearDown() {
	userlogc((char*) "TestTPACall::tearDown");

	if(unadvertised == false) {
		int toCheck = tpunadvertise((char*) "TestOne");
		userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
		BT_ASSERT(tperrno == 0);
		BT_ASSERT(toCheck != -1);
	}

	// Clean up server
	BaseServerTest::tearDown();
}

void TestExternManageDestination::test_tpcall_with_service() {
	userlogc((char*) "test_tpcall_with_service");

	char msg[80];
	char* buf = (char*) "test";
	long  sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, 0);

	sprintf(msg, "%d %d (%s)", tperrno, cd, recvbuf);
	BT_ASSERT_MESSAGE(msg, cd != -1);
	BT_ASSERT_MESSAGE(msg, recvlen == 8);
	BT_ASSERT_MESSAGE(msg, strncmp(recvbuf, "testone", 7) == 0);

	tpfree(sendbuf);
	tpfree(recvbuf);
}

void TestExternManageDestination::test_tpcall_without_service() {
	userlogc((char*) "test_tpcall_without_service");

	char* buf = (char*) "test";
	long  sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	tpunadvertise((char*) "TestOne");
	unadvertised = true;
	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, 0);
	userlogc((char*) "test_tpcall_without_service %d %d", cd, tperrno);
	BT_ASSERT(cd == -1);
	/*
	 * We don't return TPENOENT since we allow queuing even if the service is temporarily down.
	 */
	BT_ASSERT(tperrno == TPETIME);
}

void test_extern_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_extern_service");
	int len = 8;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "testone");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}
