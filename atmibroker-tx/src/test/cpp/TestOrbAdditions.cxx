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
#include "TestOrbAdditions.h"
#include "Worker.h"
#include "tx.h"
#include "AtmiBrokerOTS.h"
#include "OrbManagement.h"
#include "AtmiBrokerPoaFac.h"

void TestOrbAdditions::test_initorb() {

	for (int i = 0; i < 10; i++) {

		Worker* server_worker;
		CORBA::ORB_var server_orb;
		CosNaming::NamingContextExt_var server_default_context;
		CosNaming::NamingContext_var server_name_context;
		PortableServer::POA_var server_root_poa;
		PortableServer::POAManager_var server_root_poa_manager;
		AtmiBrokerOTS::init_orb((char*) "server", server_worker, server_orb, server_default_context, server_name_context);
		getRootPOAAndManager(server_orb, server_root_poa, server_root_poa_manager);
		AtmiBrokerPoaFac* server_poa_fac = new AtmiBrokerPoaFac();
		PortableServer::POA_var server_poa = server_poa_fac->createCallbackPoa(server_orb, "foo", server_root_poa, server_root_poa_manager);

		Worker* client_worker;
		CORBA::ORB_var client_orb;
		CosNaming::NamingContextExt_var client_default_context;
		CosNaming::NamingContext_var client_name_context;
		PortableServer::POA_var client_root_poa;
		PortableServer::POAManager_var client_root_poa_manager;
		AtmiBrokerOTS::init_orb((char*) "client", client_worker, client_orb, client_default_context, client_name_context);
		getRootPOAAndManager(client_orb, client_root_poa, client_root_poa_manager);
		AtmiBrokerPoaFac* client_poa_fac = new AtmiBrokerPoaFac();
		PortableServer::POA_var client_poa = client_poa_fac->createCallbackPoa(client_orb, "foo.client", client_root_poa, client_root_poa_manager);

		shutdownBindings(client_orb, client_root_poa, client_root_poa_manager, client_default_context, client_name_context, client_poa, client_worker);
		shutdownBindings(server_orb, server_root_poa, server_root_poa_manager, server_default_context, server_name_context, server_poa, server_worker);
	}

}
