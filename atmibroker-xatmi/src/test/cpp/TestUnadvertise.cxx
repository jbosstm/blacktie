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

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "xatmi.h"
#include "userlogc.h"
#include "TestUnadvertise.h"

void TestUnadvertise::setUp() {
	userlogc((char*) "TestUnadvertise::setUp");
	BaseServerTest::setUp();
}

void TestUnadvertise::tearDown() {
	userlogc((char*) "TestUnadvertise::tearDown");
	BaseServerTest::tearDown();
}

int TestUnadvertise::calladmin(char* command, char expect) {
	long  sendlen = strlen(command) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, command);

	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "default_ADMIN_1", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, TPNOTRAN);
	CPPUNIT_ASSERT(recvlen == 1);
	CPPUNIT_ASSERT(recvbuf[0] == expect);

	return cd;
}

int TestUnadvertise::callBAR() {
	long  sendlen = strlen((char*)"test") + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, (char*) "test");

	int cd = ::tpacall((char*) "BAR", (char *) sendbuf, sendlen, TPNOREPLY);
	return cd;
}

void TestUnadvertise::testAdminService() {
	int cd;

	// should not unadvertise ADMIN service by itself
	cd = calladmin((char*)"unadvertise,default_ADMIN_1", '0');
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestUnadvertise::testUnknowService() {
	int   cd;

	cd = calladmin((char*)"unadvertise,UNKNOW,", '0');
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestUnadvertise::testUnadvertise() {
	int   cd;

	userlogc((char*) "tpacall BAR before unadvertise");
	cd = callBAR();
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
	
	cd = calladmin((char*)"unadvertise,BAR,", '1');
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);

	userlogc((char*) "tpacall BAR after unadvertise");
	cd = callBAR();
	CPPUNIT_ASSERT(cd != 0);
	CPPUNIT_ASSERT(tperrno != 0);
}
