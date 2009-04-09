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

#include "TestClientInit.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"

#include "xatmi.h"
extern "C" {
#include "AtmiBrokerClientControl.h"
}
static char  orig_env[256];

void TestClientInit::test_clientinit() {
	CPPUNIT_ASSERT(tperrno == 0);
	int valToTest = ::clientinit();
	CPPUNIT_ASSERT(valToTest != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	valToTest = ::clientdone();
	CPPUNIT_ASSERT(valToTest != -1);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestClientInit::test_config_env() {
	char* env;

	env = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");
	if(env != NULL){
		ACE_OS::snprintf(orig_env, 256, "BLACKTIE_CONFIGURATION_DIR=%s", env);
	}

#ifdef WIN32
	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=win32");
#else
	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=linux");
#endif

	CPPUNIT_ASSERT(tperrno == 0);
	int valToTest = ::clientinit();
	CPPUNIT_ASSERT(valToTest != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	valToTest = ::clientdone();
	CPPUNIT_ASSERT(valToTest != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	/* wrong envionment */
	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=nosuch_conf");
	valToTest = ::clientinit();
	CPPUNIT_ASSERT(valToTest == -1);

	if(env != NULL) {
		ACE_OS::putenv(orig_env);
	} else {
		ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=");
	}
}
