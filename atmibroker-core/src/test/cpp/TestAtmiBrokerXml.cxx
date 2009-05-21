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
	struct ServiceMetadata* service = &data.serviceDatas[0];
	CPPUNIT_ASSERT(service);
	CPPUNIT_ASSERT(service->name == "BAR");
	CPPUNIT_ASSERT(service->function_name == "bar");
	CPPUNIT_ASSERT(service->library_name == "libBar.so");
	CPPUNIT_ASSERT(service->advertised);
	CPPUNIT_ASSERT(service->transport == "libatmibroker-corba.so");

	service = &data.serviceDatas[1];
	CPPUNIT_ASSERT(service);
	CPPUNIT_ASSERT(service->name == "FOO");
	CPPUNIT_ASSERT(service->function_name == "foo");
	CPPUNIT_ASSERT(service->library_name == "");
	CPPUNIT_ASSERT(service->advertised == false);
	CPPUNIT_ASSERT(service->transport == "libatmibroker-stomp.so");
}

void TestAtmiBrokerXml::test_client() {
	AtmiBrokerClientXml xml;
	std::vector<ClientServerInfo*> aClientServerVectorPtr;

	CPPUNIT_ASSERT(xml.parseXmlDescriptor(&aClientServerVectorPtr, "Environment.xml"));
	//CPPUNIT_ASSERT(clientMaxChannels == 1 );
	//CPPUNIT_ASSERT(clientMaxSuppliers == 1 );
	//CPPUNIT_ASSERT(clientMaxConsumers == 1 );

	ClientServerInfo* server = aClientServerVectorPtr[1];
	CPPUNIT_ASSERT(server != NULL);
	CPPUNIT_ASSERT(strcmp(server->serverName, "foo") == 0);
	std::vector<ClientServiceInfo>* services = &server->serviceVector;
	CPPUNIT_ASSERT(strcmp((*services)[0].serviceName, "BAR") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib, "atmibroker-corba.so") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib, "atmibroker-stomp.so") == 0);

	server = aClientServerVectorPtr[0];
	CPPUNIT_ASSERT(strcmp(server->serverName, "default") == 0);

	for (std::vector<ClientServerInfo*>::iterator itServer = aClientServerVectorPtr.begin(); itServer != aClientServerVectorPtr.end(); itServer++) {
		free((*itServer)->serverName);

		std::vector<ClientServiceInfo>* services = &(*itServer)->serviceVector;
		for(std::vector<ClientServiceInfo>::iterator i = services->begin(); i != services->end(); i++) {
			free((*i).serviceName);
			free((*i).transportLib);
		}

		free(*itServer);
	}
}

void TestAtmiBrokerXml::test_service() {
	AtmiBrokerServiceXml xml;
	ServiceInfo service;

	xml.parseXmlDescriptor(&service, "BAR");
	CPPUNIT_ASSERT(service.poolSize == 5);
	CPPUNIT_ASSERT(strcmp(service.serviceName, "BAR") == 0);
	CPPUNIT_ASSERT(strcmp(service.function_name, "BAR") == 0);
	CPPUNIT_ASSERT(strcmp(service.library_name, "libBAR.so") == 0);
	CPPUNIT_ASSERT(service.advertised == false);

	free(service.serviceName);
	free(service.function_name);
	free(service.library_name);
}

void TestAtmiBrokerXml::test_env() {
	char* value;

	value = AtmiBrokerEnv::get_instance()->getenv((char*)"ORBOPT");
	CPPUNIT_ASSERT(strcmp(value, "-ORBInitRef NameService=corbaloc::localhost:3528/NameService") == 0);
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
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib, "atmibroker-corba.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[0].library_name, "BAR.dll") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib, "atmibroker-stomp.dll") == 0);
#else
	CPPUNIT_ASSERT(strcmp((*services)[0].transportLib, "libatmibroker-corba.so") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[0].library_name, "libBAR.so") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].serviceName, "ECHO") == 0);
	CPPUNIT_ASSERT(strcmp((*services)[1].transportLib, "libatmibroker-stomp.so") == 0);
#endif
	CPPUNIT_ASSERT((*services)[0].advertised == false);


	char* transport = AtmiBrokerEnv::get_instance()->getTransportLibrary((char*)"BAR");
#ifdef WIN32
	CPPUNIT_ASSERT(strcmp(transport, "atmibroker-corba.dll") == 0);
#else
	CPPUNIT_ASSERT(strcmp(transport, "libatmibroker-corba.so") == 0);
#endif

	AtmiBrokerEnv::discard_instance();
}
