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
#include "xatmi.h"
#include "userlogc.h"
#include "TestUnadvertise.h"

void TestUnadvertise::setUp() {
	userlogc((char*) "TestUnadvertise::setUp");

	char* argv[] = {(char*)"./server", (char*)"-i", (char*)"1", (char*)"foo"};
	int argc = sizeof(argv)/sizeof(char*);

	int initted = serverinit(argc, argv);
	// Check that there is no error on server setup
	CPPUNIT_ASSERT(initted != -1);
	CPPUNIT_ASSERT(tperrno == 0);

}

void TestUnadvertise::tearDown() {
	userlogc((char*) "TestUnadvertise::tearDown");
	serverdone();

	clientdone();
	CPPUNIT_ASSERT(tperrno == 0);
}

int TestUnadvertise::calladmin(char* command) {
	long  sendlen = strlen(command) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, command);

	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "foo_ADMIN_1", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, TPNOTRAN);
	CPPUNIT_ASSERT(recvlen == 1);
	CPPUNIT_ASSERT((recvbuf[0] == '1') || (recvbuf[0] == '0'));

	return cd;
}

int TestUnadvertise::callBAR() {
	long  sendlen = strlen((char*)"test") + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, (char*) "test");

	int cd = ::tpacall((char*) "BAR", (char *) sendbuf, sendlen, TPNOREPLY);
	return cd;
}

void TestUnadvertise::testUnknowService() {
	int   cd;

	cd = calladmin((char*)"unadvertise,UNKNOW,");
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestUnadvertise::testUnadvertise() {
	int   cd;

	userlogc((char*) "tpacall BAR before unadvertise");
	cd = callBAR();
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);
	
	cd = calladmin((char*)"unadvertise,BAR,");
	CPPUNIT_ASSERT(cd == 0);
	CPPUNIT_ASSERT(tperrno == 0);

	userlogc((char*) "tpacall BAR after unadvertise");
	cd = callBAR();
	CPPUNIT_ASSERT(cd != 0);
	CPPUNIT_ASSERT(tperrno != 0);
}
