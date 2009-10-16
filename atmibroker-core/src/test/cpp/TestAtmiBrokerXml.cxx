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

#include "malloc.h"
#include <string.h>
void TestAtmiBrokerXml::setUp() {
	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=xmltest");
	AtmiBrokerEnv::discard_instance();

	// Perform global set up
	TestFixture::setUp();
}

void TestAtmiBrokerXml::tearDown() {
	// Perform clean up
	ACE_OS::putenv("BLACKTIE_CONFIGURATION_DIR=");
	AtmiBrokerEnv::discard_instance();

	// Perform global clean up
	TestFixture::tearDown();
}

void TestAtmiBrokerXml::test_service() {
	AtmiBrokerServiceXml xml;
	ServiceInfo service;

	xml.parseXmlDescriptor(&service, "XMLTESTSERVICE", "xmltest",
			(char*) "xmltest");
	CPPUNIT_ASSERT(service.poolSize == 5);
	CPPUNIT_ASSERT(strcmp(service.function_name, "BAR") == 0);
	CPPUNIT_ASSERT(strcmp(service.library_name, "libXMLTESTSERVICE.so") == 0);
	CPPUNIT_ASSERT(service.advertised == false);

	free(service.function_name);
	free(service.library_name);
}

void TestAtmiBrokerXml::test_env() {
	userlogc((char*) "RUNNING");
	AtmiBrokerEnv::set_configuration("xmltest");
	char* value;
	value = AtmiBrokerEnv::get_instance()->getenv((char*) "MYLIBTEST");
	CPPUNIT_ASSERT(strcmp(value, "xmltestfoo.xmltest") == 0);

	value = orbConfig.opt;
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
	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[2].serviceName, "foo_ADMIN") == 0);
#ifdef WIN32
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib, "atmibroker-hybrid.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib, "atmibroker-hybrid.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[2].transportLib, "atmibroker-hybrid.dll") == 0);
#else
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib,
			"libatmibroker-hybrid.so") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib,
			"libatmibroker-hybrid.so") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[2].transportLib,
			"libatmibroker-hybrid.so") == 0);
#endif

	char* transport = AtmiBrokerEnv::get_instance()->getTransportLibrary(
			(char*) "BAR");
#ifdef WIN32
	CPPUNIT_ASSERT(strcmp(transport, "atmibroker-hybrid.dll") == 0);
#else
	CPPUNIT_ASSERT(strcmp(transport, "libatmibroker-hybrid.so") == 0);
#endif

	CPPUNIT_ASSERT(buffers.size() == 2);

	char* foo = (char*) "foo";
	Buffer* foob = buffers[foo];
	CPPUNIT_ASSERT(strcmp(foob->name, "foo") == 0);
	CPPUNIT_ASSERT(foob->wireSize == ((4 * 3) + 8 + (1 * 2 * 10)));
	CPPUNIT_ASSERT(foob->memSize == sizeof(FOO));
	CPPUNIT_ASSERT(foob->attributes.size() == 3);
	char* Balance2 = (char*) "Balance2";
	char* Balance = (char*) "Balance";
	char* accountName = (char*) "accountName";
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance2]->id, "Balance2") == 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance2]->type, "float[]") == 0);
	CPPUNIT_ASSERT(foob->attributes[Balance2]->count == 3);
	CPPUNIT_ASSERT(foob->attributes[Balance2]->length == 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance2]->defaultValue, "1") == 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[accountName]->id, "accountName")
			== 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[accountName]->type, "char[][]") == 0);
	CPPUNIT_ASSERT(foob->attributes[accountName]->count == 2);
	CPPUNIT_ASSERT(foob->attributes[accountName]->length == 10);
	CPPUNIT_ASSERT(strcmp(foob->attributes[accountName]->defaultValue, "foo")
			== 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance]->id, "Balance") == 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance]->type, "long") == 0);
	CPPUNIT_ASSERT(foob->attributes[Balance]->count == 0);
	CPPUNIT_ASSERT(foob->attributes[Balance]->length == 0);
	CPPUNIT_ASSERT(strcmp(foob->attributes[Balance]->defaultValue, "12") == 0);

	char* bar = (char*) "bar";
	Buffer* barb = buffers[bar];
	CPPUNIT_ASSERT(strcmp(barb->name, "bar") == 0);
	CPPUNIT_ASSERT(barb->wireSize == ((4 * 4) + (2) + (4 * 4) + (2)));
	CPPUNIT_ASSERT(barb->memSize == sizeof(BAR));
	CPPUNIT_ASSERT(barb->attributes.size() == 4);
	char* barlance = (char*) "barlance";
	char* barbq = (char*) "barbq";
	char* barlance1 = (char*) "barlance1";
	char* barbq2 = (char*) "barbq2";
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance]->id, "barlance") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance]->type, "int[]") == 0);
	CPPUNIT_ASSERT(barb->attributes[barlance]->count == 4);
	CPPUNIT_ASSERT(barb->attributes[barlance]->length == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance]->defaultValue, "1") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq]->id, "barbq") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq]->type, "short") == 0);
	CPPUNIT_ASSERT(barb->attributes[barbq]->count == 0);
	CPPUNIT_ASSERT(barb->attributes[barbq]->length == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq]->defaultValue, "2") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance1]->id, "barlance1") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance1]->type, "int[]") == 0);
	CPPUNIT_ASSERT(barb->attributes[barlance1]->count == 4);
	CPPUNIT_ASSERT(barb->attributes[barlance1]->length == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barlance1]->defaultValue, "1") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq2]->id, "barbq2") == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq2]->type, "short") == 0);
	CPPUNIT_ASSERT(barb->attributes[barbq2]->count == 0);
	CPPUNIT_ASSERT(barb->attributes[barbq2]->length == 0);
	CPPUNIT_ASSERT(strcmp(barb->attributes[barbq2]->defaultValue, "2") == 0);

	Buffers::iterator it;
	for (it = buffers.begin(); it != buffers.end(); ++it) {
		Buffer* buffer = it->second;
		userlogc((char*) "Buffer name: %s", buffer->name);
	}

	AtmiBrokerEnv::discard_instance();

}
