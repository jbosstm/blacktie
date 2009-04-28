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
#include "TestServerinit.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "AtmiBrokerServer.h"

extern void test_service(TPSVCINFO *svcinfo);
static char  orig_env[256];

void TestServerinit::test_serverinit() {
	int result;

	result = serverinit(0, NULL);
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	CPPUNIT_ASSERT(ptrServer->isAdvertised((char*)"BAR"));
	result = serverdone();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);
}

void TestServerinit::test_config_env() {
	int result;
	char* argv[] = {(char*)"server"};
	int argc = sizeof(argv)/sizeof(char*);
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
	result = serverinit(argc, argv);
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	result = serverdone();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=nosuch_conf");
	result = serverinit(argc, argv);
	CPPUNIT_ASSERT(result == -1);

	if(env != NULL) {
		ACE_OS::putenv(orig_env);
	} else {
		ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=");
	}
}

void TestServerinit::test_config_cmdline() {
	int result;

#ifdef WIN32
		char* argv1[] = {(char*)"server", (char*)"-c", (char*)"win32"};
#else
		char* argv1[] = {(char*)"server", (char*)"-c", (char*)"linux"};
#endif
	int argc1 = sizeof(argv1)/sizeof(char*);

	result = serverinit(argc1, argv1);
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	int id = ::tpadvertise((char*) "TestTPAdvertise", test_service);
	CPPUNIT_ASSERT(tperrno == 0);
	CPPUNIT_ASSERT(id == 0);

	result = serverdone();
	CPPUNIT_ASSERT(result != -1);
	CPPUNIT_ASSERT(tperrno == 0);

	/* invalid command line arguments */
	char* argv2[] = {(char*)"server", (char*)"-i", (char*)"conf"};
	int argc2 = sizeof(argv2)/sizeof(char*);

	result = serverinit(argc2, argv2);
	CPPUNIT_ASSERT(result == -1);
}
void test_service(TPSVCINFO *svcinfo) {
}
