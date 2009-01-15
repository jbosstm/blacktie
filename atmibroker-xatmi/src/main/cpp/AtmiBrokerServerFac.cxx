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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#include <iostream>

#include "AtmiBrokerEnvXml.h"
#include "AtmiBrokerServerFac.h"
#include "AtmiBroker.h"

#include "userlog.h"
#include "AtmiBrokerServer.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServerFac(Logger::getLogger("AtmiBrokerServerFac"));

AtmiBrokerServerFac::AtmiBrokerServerFac() {
}

AtmiBrokerServerFac::~AtmiBrokerServerFac() {
}

void AtmiBrokerServerFac::createServer(int argc, char** argv) {
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() started ");

	CORBA::Object_var tmp_ref;
	CosNaming::Name * name;

	server_root_poa_manager->activate();
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "activated poa");

	// Create a reference for interface AtmiBroker::Server.
	ptrServer = new AtmiBroker_ServerImpl(server_poa);
	PortableServer::ObjectId_var oid = PortableServer::string_to_ObjectId(server);
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() created oid");
	server_poa->activate_object_with_id(oid, ptrServer);
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() activated object");
	// TODO SHOULD GET FROM ID
	tmp_ref = server_poa->create_reference_with_id(oid, "IDL:AtmiBroker/Server:1.0");
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() created reference");

	name = server_default_context->to_name(ptrServer->getServerName());
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() created name");
	server_name_context->rebind(*name, tmp_ref);
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() rebound");
	pServer = AtmiBroker::Server::_narrow(tmp_ref);
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() narrowed");

	CORBA::String_var ior = server_orb->object_to_string(pServer);
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() Server ior %s", (char*) ior);

	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer() created AtmiBroker::Server %p", (void*) pServer);

	// initialize the CORBA Server
	userlog(Level::getDebug(), loggerAtmiBrokerServerFac, (char*) "createServer()Initializing the CORBA Server ");

	ptrServer->serverinit(argc, argv);

	userlog(Level::getInfo(), loggerAtmiBrokerServerFac, (char*) "createServer()Initialized the CORBA Server '%s'", ptrServer->getServerName());
}
