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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


#ifndef AtmiBroker_CLIENT_H_
#define AtmiBroker_CLIENT_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>
#include <tao/PortableServer/PortableServerC.h>
#include "AtmiBrokerC.h"
#endif

#include "AtmiBrokerClientXml.h"
#include "AtmiBrokerPoaFac.h"
#include "EndpointQueue.h"

#include <iostream>
#include <vector>

class ATMIBROKER_DLL AtmiBrokerClient {
public:

	AtmiBrokerClient();
	virtual ~AtmiBrokerClient();
	AtmiBroker::ServiceQueue_ptr get_service_queue(const char * serviceName);
	EndpointQueue * getLocalCallback(int id);
	EndpointQueue * getRemoteCallback(int id);
protected:

	std::vector<ClientServerInfo*> clientServerVector;
	AtmiBroker::EndpointQueue_var clientCallback;
	EndpointQueue * clientCallbackImpl;
};

extern ATMIBROKER_DLL CORBA::ORB_var client_orb;
extern ATMIBROKER_DLL PortableServer::POA_var client_root_poa;
extern ATMIBROKER_DLL PortableServer::POAManager_var client_root_poa_manager;
extern ATMIBROKER_DLL CosNaming::NamingContextExt_var client_default_context;
extern ATMIBROKER_DLL CosNaming::NamingContext_var client_name_context;
extern ATMIBROKER_DLL PortableServer::POA_var client_poa;
extern ATMIBROKER_DLL AtmiBrokerPoaFac * clientPoaFactory;

extern ATMIBROKER_DLL AtmiBrokerClient * ptrAtmiBrokerClient;
extern ATMIBROKER_DLL CORBA::PolicyList *policyList;
#endif
