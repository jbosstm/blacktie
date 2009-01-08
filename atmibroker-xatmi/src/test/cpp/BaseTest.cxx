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

#include "BaseTest.h"

#include "xatmi.h"
extern "C" {
#include "AtmiBrokerClientControl.h"
}

void BaseTest::setUp() {
	// Perform global set up
	TestFixture::setUp();

	// Perform set up
	startNamingService();
}

void BaseTest::tearDown() {
	// Perform clean up
	::clientdone();
	CPPUNIT_ASSERT(tperrno == 0);
	stopNamingService();

	// Perform global clean up
	TestFixture::tearDown();
}

void BaseTest::startNamingService() {
	// TODO THIS ASSUMES CERTAIN LOCATIONS OF FILES
	//m_pid = exec("/home/tom/blacktie/utils/ACE+TAO-5.6.6/ACE_wrappers/TAO/orbsvcs/Naming_Service/Naming_Service", "-o namingService.ior -m 1 -u namingServiceData -ORBEndPoint iiop://localhost:3528");
	//CPPUNIT_ASSERT(m_pid != -1);
}

void BaseTest::stopNamingService() {
	//CPPUNIT_ASSERT(term(m_pid) != -1);
}
