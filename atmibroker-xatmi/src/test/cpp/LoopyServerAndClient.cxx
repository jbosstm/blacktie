/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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

#include "LoopyServerAndClient.h"

#include "xatmi.h"
#include "AtmiBrokerServerControl.h"
#include "AtmiBrokerClientControl.h"

void LoopyServerAndClient::setUp() {
	// Perform global set up
	TestFixture::setUp();
}

void LoopyServerAndClient::tearDown() {
	// Perform global clean up
	TestFixture::tearDown();
}

void LoopyServerAndClient::testLoopy() {
	int result = 0;
	int argc = 0;
	char** argv = NULL;

	result = serverinit(argc, argv);
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	result = clientinit();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	result = clientdone();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	result = serverdone();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);
}
