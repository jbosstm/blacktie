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

#include "TestAtmiBrokerXml.h"
#include "AtmiBrokerServiceXml.h"
#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerEnv.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "userlogc.h"

void TestAtmiBrokerXml::setUp() {
	env = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");

	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=xmltest");
	AtmiBrokerEnv::discard_instance();

	// Perform global set up
	TestFixture::setUp();
}

void TestAtmiBrokerXml::tearDown() {
	// Perform clean up
	if (env != NULL) {
		char orig_env[256];
		ACE_OS::snprintf(orig_env, 256, "BLACKTIE_CONFIGURATION_DIR=%s", env);
		ACE_OS::putenv(orig_env);
	} else {
		ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=");
	}

	AtmiBrokerEnv::discard_instance();

	// Perform global clean up
	TestFixture::tearDown();
}

void TestAtmiBrokerXml::test_service() {
	AtmiBrokerServiceXml xml;
	ServiceInfo service;

	xml.parseXmlDescriptor(&service, "BAR", "xmltest/foo");
	CPPUNIT_ASSERT(service.poolSize == 5);
	CPPUNIT_ASSERT(strcmp(service.function_name, "BAR") == 0);
	CPPUNIT_ASSERT(strcmp(service.library_name, "libBAR.so") == 0);
	CPPUNIT_ASSERT(service.advertised == false);

	free(service.function_name);
	free(service.library_name);
}

void TestAtmiBrokerXml::test_env() {
	userlogc((char*) "RUNNING");
	char* value;
	value = AtmiBrokerEnv::get_instance()->getenv((char*) "MYLIBTEST");
	CPPUNIT_ASSERT(strcmp(value, "xmltestfoo.xmltest") == 0);

	value = AtmiBrokerEnv::get_instance()->getenv((char*) "ORBOPT");
	CPPUNIT_ASSERT(strncmp(value, "-ORBInitRef NameService=corbaloc::", 34)
			== 0);
	CPPUNIT_ASSERT(strcmp(domain, "fooapp") == 0);
	CPPUNIT_ASSERT(xarmp != 0);

	CPPUNIT_ASSERT(servers.size() == 2);
	ServerInfo* server = servers[1];
	CPPUNIT_ASSERT(server != NULL);
	CPPUNIT_ASSERT(strcmp(server->serverName, "foo") == 0);
	std::vector<ServiceInfo>* services = &server->serviceVector;
	CPPUNIT_ASSERT(strcmp((*services)[0].serviceName, "BAR") == 0);
	CPPUNIT_ASSERT((*services)[0].poolSize == 5);
	CPPUNIT_ASSERT(strcmp((*services)[0].function_name, "BAR") == 0);
#ifdef WIN32
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib, "atmibroker-hybrid.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[0].library_name, "BAR.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib, "atmibroker-hybrid.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[2].serviceName, "foo_ADMIN") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[2].transportLib, "atmibroker-hybrid.dll") == 0);
#else

	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib,
			"libatmibroker-hybrid.so") == 0);

	CPPUNIT_ASSERT(strcmp((*services)[0].library_name, "libBAR.so") == 0);

	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);

	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib,
			"libatmibroker-hybrid.so") == 0);

	CPPUNIT_ASSERT(strcmp((*services)[2].serviceName, "foo_ADMIN") == 0);

	CPPUNIT_ASSERT(strcmp((*services)[2].transportLib,
			"libatmibroker-hybrid.so") == 0);

#endif
	CPPUNIT_ASSERT((*services)[0].advertised == false);

	char* transport = AtmiBrokerEnv::get_instance()->getTransportLibrary(
			(char*) "BAR");
#ifdef WIN32
	CPPUNIT_ASSERT(strcmp(transport, "atmibroker-hybrid.dll") == 0);
#else
	CPPUNIT_ASSERT(strcmp(transport, "libatmibroker-hybrid.so") == 0);
#endif

	AtmiBrokerEnv::discard_instance();

}
