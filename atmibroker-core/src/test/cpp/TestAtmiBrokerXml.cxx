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
#include "ace/OS_NS_stdlib.h"

void TestAtmiBrokerXml::setUp()
{
	ACE_OS::putenv("ATMIBROKER_SCHEMA_DIR=./xsd");
}

void TestAtmiBrokerXml::test_server() {
	AtmiBrokerServerXml xml;
	ServerMetadata      data;

	xml.parseXmlDescriptor(&data, "SERVER.xml");
	CPPUNIT_ASSERT(data.maxChannels == 1);
	CPPUNIT_ASSERT(data.maxSuppliers == 1);
	CPPUNIT_ASSERT(data.maxConsumers == 1);
	//CPPUNIT_ASSERT(data.maxReplicas == 1);
	CPPUNIT_ASSERT(data.orbType == "TAO");
	CPPUNIT_ASSERT(data.serviceNames[0] == "BAR");
}

void TestAtmiBrokerXml::test_client() {
}

void TestAtmiBrokerXml::test_service() {
}

void TestAtmiBrokerXml::test_env() {
}
