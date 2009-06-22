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

#include "malloc.h"

int interationCount = 100;

extern void testTPConversation_service(TPSVCINFO *svcinfo);

void TestTPConversation::setUp() {
	userlogc((char*) "TestTPConversation::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	cd = -1;
	int toCheck = tpadvertise((char*) "TestTPConversation",
			testTPConversation_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	sendlen = 11;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT(
			(rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sendlen))
					!= NULL);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTPConversation::tearDown() {
	userlogc((char*) "TestTPConversation::tearDown");
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
	userlogc((char*) "test_conversation");
	strcpy(sendbuf, "conversate");
	cd
			= ::tpconnect((char*) "TestTPConversation", sendbuf, sendlen,
					TPRECVONLY);
	long revent = 0;
	char* tperrnoS = (char*) malloc(110);
	sprintf(tperrnoS, "%d", tperrno);
	CPPUNIT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	CPPUNIT_ASSERT(cd != -1);
	userlogc("Started conversation");
	for (int i = 0; i < interationCount; i++) {
		int result = ::tprecv(cd, &rcvbuf, &rcvlen, 0, &revent);
		sprintf(tperrnoS, "%d", tperrno);
		CPPUNIT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
		CPPUNIT_ASSERT(result != -1);
		char* expectedResult = (char*) malloc(sendlen);
		sprintf(expectedResult, "hi%d", i);
		char* errorMessage = (char*) malloc(sendlen * 2 + 1);
		sprintf(errorMessage, "%s/%s", expectedResult, rcvbuf);
		CPPUNIT_ASSERT_MESSAGE(errorMessage, strcmp(expectedResult, rcvbuf)
				== 0);
		free(expectedResult);
		free(errorMessage);

		sprintf(sendbuf, "yo%d", i);
		//userlogc((char*) "test_conversation:%s:", sendbuf);
		result = ::tpsend(cd, sendbuf, sendlen, TPRECVONLY, &revent);
		sprintf(tperrnoS, "%d", tperrno);
		CPPUNIT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
		CPPUNIT_ASSERT(result != -1);
	}
	userlogc("Conversed");
	int result = ::tpgetrply(&cd, &rcvbuf, &rcvlen, 0);
	sprintf(tperrnoS, "%d", tperrno);
	CPPUNIT_ASSERT_MESSAGE(tperrnoS, tperrno == 0);
	CPPUNIT_ASSERT(result != -1);
	free(tperrnoS);

	char* expectedResult = (char*) malloc(sendlen);
	sprintf(expectedResult, "hi%d", interationCount);
	char* errorMessage = (char*) malloc(sendlen * 2 + 1);
	sprintf(errorMessage, "%s/%s", expectedResult, rcvbuf);
	CPPUNIT_ASSERT_MESSAGE(errorMessage, strcmp(expectedResult, rcvbuf) == 0);
	free(expectedResult);
	free(errorMessage);
}

void testTPConversation_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testTPConversation_service");
	bool fail = false;
	char *sendbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);
	char *rcvbuf = ::tpalloc((char*) "X_OCTET", NULL, svcinfo->len);

	char* expectedResult = (char*) malloc(svcinfo->len);
	strcpy(expectedResult, "conversate");
	char* errorMessage = (char*) malloc(svcinfo->len * 2 + 1);
	sprintf(errorMessage, "%s/%s", expectedResult, svcinfo->data);
	if (strcmp(expectedResult, svcinfo->data) != 0) {
		if (svcinfo->data != NULL) {
			userlogc("Got invalid data %s", svcinfo->data);
		} else {
			userlogc("GOT A NULL");
		}
		fail = true;
	} else {
		long revent = 0;
		userlogc("Chatting");
		for (int i = 0; i < interationCount; i++) {
			sprintf(sendbuf, "hi%d", i);
			//userlogc((char*) "testTPConversation_service:%s:", sendbuf);
			int result = ::tpsend(svcinfo->cd, sendbuf, svcinfo->len,
					TPRECVONLY, &revent);
			if (result != -1) {
				result = ::tprecv(svcinfo->cd, &rcvbuf, &svcinfo->len, 0, &revent);
				if (result != -1) {
					char* expectedResult = (char*) malloc(svcinfo->len);
					sprintf(expectedResult, "yo%d", i);
					char* errorMessage = (char*) malloc(svcinfo->len * 2 + 1);
					sprintf(errorMessage, "%s/%s", expectedResult, rcvbuf);
					if (strcmp(expectedResult, rcvbuf) != 0) {
						fail = true;
						break;
					}
				} else {
					fail = true;
					break;
				}
			} else {
				fail = true;
				break;
			}
		}
		userlogc("Chatted");
	}

	if (fail) {
		tpreturn(TPESVCFAIL, 0, sendbuf, 0, 0);
	} else {
		sprintf(sendbuf, "hi%d", interationCount);
		tpreturn(TPSUCCESS, 0, sendbuf, svcinfo->len, 0);
	}

	::tpfree(rcvbuf);
	free(expectedResult);
	free(errorMessage);
}
