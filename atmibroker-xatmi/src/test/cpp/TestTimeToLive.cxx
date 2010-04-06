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

#include "Sleeper.h"
#include "xatmi.h"
#include "userlogc.h"
#include "TestTimeToLive.h"

void test_TTL_service(TPSVCINFO *svcinfo) {
	long timeout = 45;

	::sleeper(timeout);
	userlogc((char*) "TTL sleep timeout %d seconds", timeout);

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_tpcall_TTL_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void TestTimeToLive::setUp() {
	userlogc((char*) "TestTimeToLive::setUp");
	BaseServerTest::setUp();
}

void TestTimeToLive::tearDown() {
	userlogc((char*) "TestTimeToLive::tearDown");
	BaseServerTest::tearDown();
}

void TestTimeToLive::testTTL() {
	int rc = tpadvertise((char*) "TTL", test_TTL_service);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(rc != -1);

	int cd;
	cd = callTTL();
	BT_ASSERT(cd == -1);
	BT_ASSERT(tperrno == TPETIME);
	userlogc((char*)"send first message");

	cd = callTTL();
	BT_ASSERT(cd == -1);
	BT_ASSERT(tperrno == TPETIME);
	userlogc((char*)"send second message");

	::sleeper(30);
	long n = getTTLCounter();	
	userlogc((char*)"TTL get message counter is %d", n);
	//BT_ASSERT(n == 1);
}

int TestTimeToLive::callTTL() {
	long  sendlen = strlen((char*)"test") + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, (char*) "test");

	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "TTL", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, 0);
	return cd;
}

long TestTimeToLive::getTTLCounter() {
	long sendlen = strlen("counter,TTL,") + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, "counter,TTL,");

	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "default_ADMIN_1", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, 0);
	BT_ASSERT(cd == 0);
	BT_ASSERT(tperrno == 0);
	BT_ASSERT(recvbuf[0] == '1');

	return (atol(&recvbuf[1]));
}
