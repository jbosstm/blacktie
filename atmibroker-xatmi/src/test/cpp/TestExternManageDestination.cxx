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

#include "AtmiBrokerServerControl.h"

#include "xatmi.h"
#include "malloc.h"

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"

#if defined(__cplusplus)
extern "C" {
#endif
extern void test_extern_service(TPSVCINFO *svcinfo);
#if defined(__cplusplus)
}
#endif

static int msgCnt;

void TestExternManageDestination::setUp() {
	userlogc((char*) "TestExternManageDestination::setUp");
	unadvertised = false;

	ACE_OS::putenv("BLACKTIE_SERVER_ID=1");

	// Set up server
	BaseTest::setUp();
}

void TestExternManageDestination::tearDown() {
	userlogc((char*) "TestTPACall::tearDown");

	if (unadvertised == false) {
		int toCheck = tpunadvertise((char*) "TestOne");
		userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
		BT_ASSERT(tperrno == 0);
		BT_ASSERT(toCheck != -1);
	}

	ACE_OS::putenv("BLACKTIE_SERVER_ID=");

	// Clean up server
	BaseServerTest::tearDown();
}

void TestExternManageDestination::test_tpcall_with_service() {
	userlogc((char*) "test_tpcall_with_service");

	// Advertise the service
	int toCheck = tpadvertise((char*) "TestOne", test_extern_service);
	userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	char msg[80];
	char* buf = (char*) "test";
	long sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long recvlen = 1;

	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen,
			(char**) &recvbuf, &recvlen, 0);

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
	long sendlen = strlen(buf) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, buf);
	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long recvlen = 1;

	unadvertised = true;
	int cd = ::tpcall((char*) "TestOne", (char *) sendbuf, sendlen,
			(char**) &recvbuf, &recvlen, 0);
	userlogc((char*) "test_tpcall_without_service %d %d", cd, tperrno);
	BT_ASSERT(cd == -1);
	/*
	 * We don't return TPENOENT since we allow queuing even if the service is temporarily down.
	 */
	BT_ASSERT(tperrno == TPETIME);
}

void TestExternManageDestination::test_stored_messages() {
	for (int i = 0; i < 10; i++) {
		char msg[80];
		sprintf(msg, (char*) "request %d", i);
		long sendlen = strlen(msg) + 1;
		char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
		(void) strcpy(sendbuf, msg);
		int cd = tpacall("TestOne", sendbuf, sendlen, TPNOREPLY);

		if (cd != 0 || tperrno != 0)
			userlogc((char*) "tpacall returned %d %d", cd, tperrno);
		else
			userlogc((char*) "sent %d:", i);

		tpfree(sendbuf);
	}

	// Advertise the service
	int toCheck = tpadvertise((char*) "TestOne", test_extern_service);
	userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	msgCnt = 5;
	maxSleep = 10;
	while (msgCnt > 0 && maxSleep-- > 0)
		if (sleep(1) != 0)
			break;

	// Shutdown the server
	serverdone();

	// Advertise the service
	toCheck = tpadvertise((char*) "TestOne", test_extern_service);
	userlogc((char*) "TestExternManageDestination %d %d", toCheck, tperrno);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(toCheck != -1);

	msgCnt = 5;
	maxSleep = 10;
	while (msgCnt > 0 && maxSleep-- > 0)
		if (sleep(1) != 0)
			break;
}

void test_extern_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "test_extern_service");
	int len = 8;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "testone");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void qservice(TPSVCINFO *svcinfo) {
	userlogc((char*) "svc: %s data: %s len: %d flags: %d", svcinfo->name,
			svcinfo->data, svcinfo->len, svcinfo->flags);
	msgCnt -= 1;
}
