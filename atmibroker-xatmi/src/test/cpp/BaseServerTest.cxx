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
#include "BaseTest.h"

extern "C" {
#include "AtmiBrokerServerControl.h"
}

#include "xatmi.h"

void BaseServerTest::setUp() {
	// Perform initial start up
	BaseTest::setUp();

	int initted = serverinit();
	// Check that there is no error on server setup
	CPPUNIT_ASSERT(initted != -1);
	CPPUNIT_ASSERT(tperrno == 0);
}

void BaseServerTest::tearDown() {

	// Perform additional clean up
	BaseTest::tearDown();
	// Stop the server
	serverdone();
}
