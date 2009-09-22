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
extern "C" {
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerClientControl.h"
}

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/OS_NS_unistd.h"
#include "xatmi.h"
#include "userlogc.h"
#include "TestTimeToLive.h"

void test_TTL_service(TPSVCINFO *svcinfo) {
	long timeout = 45;

	ACE_OS::sleep(timeout);
	userlogc((char*) "TTL sleep timeout %d seconds", timeout);

	int len = 60;
	char *toReturn = ::tpalloc((char*) "X_OCTET", NULL, len);
	strcpy(toReturn, "test_tpcall_TTL_service");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

void TestTimeToLive::setUp() {
	userlogc((char*) "TestTimeToLive::setUp");

#ifdef WIN32
       char* argv[] = {(char*)"server", (char*)"-i", (char*)"1", (char*)"-c", (char*)"win32", (char*)"foo"};
#else
       char* argv[] = {(char*)"server", (char*)"-i", (char*)"1", (char*)"-c", (char*)"linux", (char*)"foo"};
#endif
	int argc = sizeof(argv)/sizeof(char*);

	int initted = serverinit(argc, argv);
	// Check that there is no error on server setup
	CPPUNIT_ASSERT(initted != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	int rc = tpadvertise((char*) "TTL", test_TTL_service);

	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(rc != -1);
}

void TestTimeToLive::tearDown() {
	userlogc((char*) "TestTimeToLive::tearDown");
	serverdone();

	clientdone();
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTimeToLive::testTTL() {
	int cd;

	cd = callTTL();
	CPPUNIT_ASSERT(cd == -1);
	CPPUNIT_ASSERT(tperrno == TPETIME);
	userlogc((char*)"send first message");

	cd = callTTL();
	CPPUNIT_ASSERT(cd == -1);
	CPPUNIT_ASSERT(tperrno == TPETIME);
	userlogc((char*)"send second message");

	ACE_OS::sleep(30);
	long n = getTTLCounter();	
	userlogc((char*)"TTL get message counter is %d", n);
	CPPUNIT_ASSERT(n == 1);
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

	int cd = ::tpcall((char*) "foo_ADMIN_1", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, 0);
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(recvbuf[0] == '1');

	return (atol(&recvbuf[1]));
}
