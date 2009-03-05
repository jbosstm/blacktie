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

#include "xatmi.h"

#include "TestTPConversation.h"

#include "userlogc.h"

int interationCount = 10000;

extern void testTPConversation_service(TPSVCINFO *svcinfo);

void TestTPConversation::setUp() {
	// Setup server
	BaseServerTest::setUp();

	// Do local work
	cd = -1;
	int toCheck = tpadvertise((char*) "TestTPConversation", testTPConversation_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	sendlen = strlen("hello") + 1;
	CPPUNIT_ASSERT((sendbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	strcpy(sendbuf, "hello");
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTPConversation::tearDown() {
	// Do local work
	::tpfree(sendbuf);
	::tpfree(rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPConversation");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPConversation::test_conversation() {
	cd = ::tpconnect((char*) "TestTPConversation", sendbuf, sendlen, TPRECVONLY);
	long revent = 0;
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(cd != -1);
	userlogc("Started conversation");
	for (int i = 0; i < interationCount; i++) {
		int result = ::tprecv(cd, &rcvbuf, &rcvlen, 0, &revent);
		CPPUNIT_ASSERT(tperrno == 0);
		CPPUNIT_ASSERT(result != -1);
		result = ::tpsend(cd, sendbuf, sendlen, TPRECVONLY, &revent);
		CPPUNIT_ASSERT(tperrno == 0);
		CPPUNIT_ASSERT(result != -1);
	}
	userlogc("Conversed");
	int result = ::tpgetrply(&cd, &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(result != -1);
}

void testTPConversation_service(TPSVCINFO *svcinfo) {
	long revent = 0;
	char *sendbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);
	strcpy(sendbuf, "hello");
	char *rcvbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);
	userlogc("Chatting");
	for (int i = 0; i < interationCount; i++) {
		int result = ::tpsend(svcinfo->cd, sendbuf, svcinfo->len, TPRECVONLY, &revent);
		result = ::tprecv(svcinfo->cd, &rcvbuf, &svcinfo->len, 0, &revent);
	}
	userlogc("Chatted");
	::tpfree(rcvbuf);
	tpreturn(TPSUCCESS, 0, sendbuf, svcinfo->len, 0);
}
