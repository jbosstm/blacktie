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
#include <cppunit/TestFixture.h>

#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>
#include <tao/PortableServer/PortableServerC.h>
#include <orbsvcs/CosNotificationS.h>

#include "SimpleOrbTest.h"
#include "Worker.h"

void SimpleOrbTest::setUp() {
	// Perform global set up
	TestFixture::setUp();

	// Perform set up
}

void SimpleOrbTest::tearDown() {
	// Perform clean up

	// Perform global clean up
	TestFixture::tearDown();
}

void SimpleOrbTest::test() {
	int argc = 2;
	char *argv[2];
	argv[0] = (char*) "-ORBInitRef";
	argv[1] = (char*) "NameService=corbaloc::localhost:3528/NameService";
	//CORBA::ORB_ptr orbRef = CORBA::ORB_init(argc, argv, "server");
	CORBA::ORB_var orbRef = CORBA::ORB_init(argc, argv, "server");
	CORBA::Object_var tmp_ref = orbRef->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(tmp_ref);
	PortableServer::POAManager_var poa_manager = poa->the_POAManager();
	//assert(!CORBA::is_nil(poa_manager));
	tmp_ref = orbRef->resolve_initial_references("NameService");
	Worker *worker = new Worker(orbRef);
	if (worker->activate(THR_NEW_LWP| THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
		delete (worker);
		worker = NULL;
	}
	try {
		CosNaming::NamingContextExt_var default_ctx = CosNaming::NamingContextExt::_narrow(tmp_ref);
	} catch (CORBA::Exception &e) {
		CPPUNIT_FAIL("COULDN'T Narrow the default context");
	}
		if (!CORBA::is_nil(orbRef))
			orbRef->shutdown(1);
		if (!CORBA::is_nil(orbRef))
			orbRef->destroy();

		if (worker != NULL) {
			worker->wait();
			delete (worker);
			worker = NULL;
		}

		orbRef = NULL;
		poa_manager = NULL;
		poa = NULL;
		CPPUNIT_ASSERT(CORBA::is_nil(orbRef));
		CPPUNIT_ASSERT(CORBA::is_nil(poa_manager));
		CPPUNIT_ASSERT(CORBA::is_nil(poa));
}
