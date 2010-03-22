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

#include "LoopyServerAndClient.h"

#include "xatmi.h"
#include "userlogc.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerClientControl.h"

void LoopyServerAndClient::setUp() {
	init_ace();
	userlogc((char*) "LoopyServerAndClient::setUp");
	// Perform global set up
	TestFixture::setUp();
}

void LoopyServerAndClient::tearDown() {
	userlogc((char*) "LoopyServerAndClient::tearDown");
	// Perform global clean up
	TestFixture::tearDown();
}

void loopy(TPSVCINFO* tpsvcinfo) {
	userlogc((char*) "loopy");
}

void LoopyServerAndClient::testLoopyAll() {
	userlogc((char*) "testLoopyAll");
	int result = 0;

	for (int i = 0; i < 3; i++) {
		result = serverinit(0, NULL);
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = clientinit();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = tpadvertise((char*) "LOOPY", loopy);
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = clientdone();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = serverdone();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);
	}
}

void LoopyServerAndClient::testLoopyAll2() {
	userlogc((char*) "testLoopyAll2");
	int result = 0;

	for (int i = 0; i < 3; i++) {
		result = serverinit(0, NULL);
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = tpadvertise((char*) "LOOPY", loopy);
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = clientinit();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = clientdone();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = serverdone();
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);
	}
}

void LoopyServerAndClient::testLoopyAdvertise() {
	userlogc((char*) "testLoopyAdvertise");
	int result = 0;
	result = serverinit(0, NULL);
	BT_ASSERT(result != -1);
	BT_ASSERT(tperrno == 0);

	result = clientinit();
	BT_ASSERT(result != -1);
	BT_ASSERT(tperrno == 0);

	for (int i = 0; i < 3; i++) {
		result = tpadvertise((char*) "LOOPY", loopy);
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);

		result = tpunadvertise((char*) "LOOPY");
		BT_ASSERT(result != -1);
		BT_ASSERT(tperrno == 0);
	}

	result = clientdone();
	BT_ASSERT(result != -1);
	BT_ASSERT(tperrno == 0);

	result = serverdone();
	BT_ASSERT(result != -1);
	BT_ASSERT(tperrno == 0);
}
