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

#include "TestTPGetRply.h"
#include "Sleeper.h"

extern void testtpgetrply_service(TPSVCINFO *svcinfo);

extern void test_tpgetrply_TPNOBLOCK(TPSVCINFO *svcinfo);

void TestTPGetRply::setUp() {
	userlogc((char*) "TestTPGetRply::setUp");
	sendbuf = NULL;
	rcvbuf = NULL;

	// Setup server
	BaseServerTest::setUp();

	// Do local work
	sendlen = strlen("grply") + 1;
	rcvlen = 22;
	CPPUNIT_ASSERT((sendbuf
			= (char *) tpalloc((char*) "X_OCTET", NULL, sendlen)) != NULL);
	CPPUNIT_ASSERT((rcvbuf = (char *) tpalloc((char*) "X_OCTET", NULL, rcvlen))
			!= NULL);
	strcpy(sendbuf, "grply");
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestTPGetRply::tearDown() {
	userlogc((char*) "TestTPGetRply::tearDown");
	// Do local work
	::tpfree( sendbuf);
	::tpfree( rcvbuf);
	int toCheck = tpunadvertise((char*) "TestTPGetrply");
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	// Clean up server
	BaseServerTest::tearDown();
}

void TestTPGetRply::test_tpgetrply() {
	userlogc((char*) "test_tpgetrply");

	int toCheck = tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	int cd = ::tpacall((char*) "TestTPGetrply", (char *) sendbuf, sendlen, 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	// RETRIEVE THE RESPONSE
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno != TPEINVAL);
	CPPUNIT_ASSERT(tperrno != TPEBADDESC);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(valToTest == 0);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "testtpgetrply_service") == 0);
}

// 8.5
void TestTPGetRply::test_tpgetrply_baddesc() {
	userlogc((char*) "test_tpgetrply_baddesc");

	int toCheck = tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	int cd = 2;
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(valToTest == -1);
	CPPUNIT_ASSERT(tperrno != 0);
	CPPUNIT_ASSERT(tperrno != TPEINVAL);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno == TPEBADDESC);
}

void TestTPGetRply::test_tpgetrply_nullcd() {
	userlogc((char*) "test_tpgetrply_nullcd");

	int toCheck = tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	int valToTest = ::tpgetrply(NULL, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(valToTest == -1);
	CPPUNIT_ASSERT(tperrno != 0);
	CPPUNIT_ASSERT(tperrno != TPEBADDESC);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
}

void TestTPGetRply::test_tpgetrply_nullrcvbuf() {
	userlogc((char*) "test_tpgetrply_nullrcvbuf");

	int toCheck = tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	int cd = ::tpacall((char*) "TestTPGetrply", (char *) sendbuf, sendlen, 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	int valToTest = ::tpgetrply(&cd, NULL, &rcvlen, 0);
	CPPUNIT_ASSERT(valToTest == -1);
	CPPUNIT_ASSERT(tperrno != 0);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno != TPEBADDESC);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
}

void TestTPGetRply::test_tpgetrply_nullrcvlen() {
	userlogc((char*) "test_tpgetrply_nullrcvlen");

	int toCheck = tpadvertise((char*) "TestTPGetrply", testtpgetrply_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(toCheck != -1);

	int cd = 2;
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, NULL, 0);
	CPPUNIT_ASSERT(valToTest == -1);
	CPPUNIT_ASSERT(tperrno != 0);
	CPPUNIT_ASSERT(tperrno != TPEBADDESC);
	CPPUNIT_ASSERT(tperrno != TPEOTYPE);
	CPPUNIT_ASSERT(tperrno != TPETIME);
	CPPUNIT_ASSERT(tperrno != TPESVCFAIL);
	CPPUNIT_ASSERT(tperrno != TPESVCERR);
	CPPUNIT_ASSERT(tperrno != TPEBLOCK);
	CPPUNIT_ASSERT(tperrno == TPEINVAL);
}

void testtpgetrply_service(TPSVCINFO *svcinfo) {
	userlogc((char*) "testtpgetrply_service");
	char * toReturn = ::tpalloc((char*) "X_OCTET", NULL, 22);
	strcpy(toReturn, "testtpgetrply_service");
	tpreturn(TPSUCCESS, 0, toReturn, 22, 0);
}

void TestTPGetRply::test_tpgetrply_with_TPNOBLOCK() {
	userlogc((char*) "test_tpgetrply_with_TPNOBLOCK");
	tpadvertise((char*) "TestTPGetrply", test_tpgetrply_TPNOBLOCK);

	int cd = ::tpacall((char*) "TestTPGetrply", (char *) sendbuf, sendlen, 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	// RETRIEVE THE RESPONSE
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, TPNOBLOCK);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(valToTest == -1);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "test_tpgetrply_TPNOBLOCK") == -1);
}

void TestTPGetRply::test_tpgetrply_without_TPNOBLOCK() {
	userlogc((char*) "test_tpgetrply_without_TPNOBLOCK");
	tpadvertise((char*) "TestTPGetrply", test_tpgetrply_TPNOBLOCK);

	int cd = ::tpacall((char*) "TestTPGetrply", (char *) sendbuf, sendlen, 0);
	CPPUNIT_ASSERT(cd != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	// RETRIEVE THE RESPONSE
	int valToTest = ::tpgetrply(&cd, (char **) &rcvbuf, &rcvlen, 0);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(valToTest == 0);
	CPPUNIT_ASSERT(strcmp(rcvbuf, "test_tpgetrply_TPNOBLOCK") == 0);
}

void test_tpgetrply_TPNOBLOCK(TPSVCINFO *svcinfo) {
	char* response = (char*) "test_tpgetrply_TPNOBLOCK";
	userlogc(response);

	long sendlen = strlen(response) + 1;
	char * toReturn = ::tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(toReturn, response);
	::sleeper(5);
	tpreturn(TPSUCCESS, 0, toReturn, sendlen, 0);
}

