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
#include "AtmiBrokerServerXml.h"
#include "AtmiBrokerClientXml.h"
#include "AtmiBrokerServiceXml.h"
#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerEnv.h"
#include "ace/OS_NS_stdlib.h"

void TestAtmiBrokerXml::test_server() {
	AtmiBrokerServerXml xml;
	ServerMetadata      data;

	CPPUNIT_ASSERT(xml.parseXmlDescriptor(&data, "SERVER.xml"));
	CPPUNIT_ASSERT(data.maxChannels == 1);
	CPPUNIT_ASSERT(data.maxSuppliers == 1);
	CPPUNIT_ASSERT(data.maxConsumers == 1);
	//CPPUNIT_ASSERT(data.maxReplicas == 1);
	CPPUNIT_ASSERT(data.orbType == "TAO");
	CPPUNIT_ASSERT(data.serviceNames[0] == "BAR");
}

void TestAtmiBrokerXml::test_client() {
	AtmiBrokerClientXml xml;
	std::vector<ClientServerInfo*> aClientServerVectorPtr;

	CPPUNIT_ASSERT(xml.parseXmlDescriptor(&aClientServerVectorPtr, "CLIENT.xml"));
	CPPUNIT_ASSERT(clientMaxChannels == 1 );
	CPPUNIT_ASSERT(clientMaxSuppliers == 1 );
	CPPUNIT_ASSERT(clientMaxConsumers == 1 );

	ClientServerInfo* server = aClientServerVectorPtr[0];
	CPPUNIT_ASSERT(strcmp(server->serverName, "foo") == 0);
	std::vector<char*>* services = server->serviceVectorPtr;
	CPPUNIT_ASSERT(strcmp((*services)[0], "BAR") == 0);

	for (std::vector<ClientServerInfo*>::iterator itServer = aClientServerVectorPtr.begin(); itServer != aClientServerVectorPtr.end(); itServer++) {
		free((*itServer)->serverName);

		std::vector<char*>* services = (*itServer)->serviceVectorPtr;
		for(std::vector<char*>::iterator i = services->begin(); i != services->end(); i++) {
			free(*i);
		}
		delete services;

		free(*itServer);
	}
}

void TestAtmiBrokerXml::test_service() {
	AtmiBrokerServiceXml xml;
	SVCINFO service;

	xml.parseXmlDescriptor(&service, "BAR");
}

void TestAtmiBrokerXml::test_env() {
	char* value;

	value = AtmiBrokerEnv::get_instance()->getenv((char*)"ORBOPT");
	CPPUNIT_ASSERT(strcmp(value, "-ORBInitRef NameService=corbaloc::localhost:3528/NameService") == 0);
	CPPUNIT_ASSERT(strcmp(domain, "fooapp") == 0);
	CPPUNIT_ASSERT(xarmp != 0);
	AtmiBrokerEnv::discard_instance();
}
