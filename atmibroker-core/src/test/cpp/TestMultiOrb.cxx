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
#include <cppunit/TestFixture.h>

#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>
#include <tao/PortableServer/PortableServerC.h>
#include <orbsvcs/CosNotificationS.h>

#include "TestMultiOrb.h"

#include "OrbManagement.h"
#include "Worker.h"

void TestMultiOrb::setUp() {
	// Perform global set up
	TestFixture::setUp();

	// Perform set up
}

void TestMultiOrb::tearDown() {
	// Perform clean up

	// Perform global clean up
	TestFixture::tearDown();
}

void TestMultiOrb::test() {
	try {
		Worker* client_worker;
		CORBA::ORB_var client_orb;
		CosNaming::NamingContextExt_var client_default_context;
		CosNaming::NamingContext_var client_name_context;
		initOrb((char*) "client", client_worker, client_orb, client_default_context, client_name_context);

		Worker* server_worker;
		CORBA::ORB_var server_orb;
		CosNaming::NamingContextExt_var server_default_context;
		CosNaming::NamingContext_var server_name_context;
		initOrb((char*) "server", server_worker, server_orb, server_default_context, server_name_context);
	} catch (CORBA::Exception &e) {
		CPPUNIT_FAIL("COULDN'T CONNECT TO NAME SERVICE");
	}
}
