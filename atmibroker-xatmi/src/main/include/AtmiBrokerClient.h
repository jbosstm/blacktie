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
#elif ORBIX_COMP
#include <omg/orb.hh>
#include <omg/CosNaming.hh>
#include <omg/PortableServer.hh>
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include <orb.h>
#include <CosNaming_c.hh>
#include <PortableServerExt_c.hh>
#include "AtmiBroker_c.hh"
#endif

#include "AtmiBrokerPoaFac.h"

#include <iostream>
#include <vector>

struct _client_server_info {

	char * serverName;
	AtmiBroker::Server_ptr serverPtr;
	std::vector<char*>* serviceVectorPtr;
	AtmiBroker::ClientInfo_var cInfo;

};

typedef _client_server_info ClientServerInfo;

class ATMIBROKER_DLL AtmiBrokerClient {
public:

	AtmiBrokerClient(CORBA::Boolean createCallback, CORBA::Boolean createChannels, CORBA::Boolean createSuppliers, CORBA::Boolean createConsumers);

	virtual ~AtmiBrokerClient();

	void getServer(ClientServerInfo * aClientServerInfo, char * serverName);

	void getService(char * serviceName, char ** id, AtmiBroker::Service_var* refPtr);

	void findService(char * serviceAndIndex, AtmiBroker::Service_var* refPtr);

	void createAndRegisterCallback(ClientServerInfo * aClientServerInfo);

	int convertIdToInt(char * id);

	char * convertIdToString(int id);

	void extractServiceAndIndex(char * serviceAndIndex, char * serviceName, char * index);

	long getClientId(char* serviceName);

	long getQueueId(char* aQueueName);

	AtmiBroker::ClientCallback_var getClientCallback();

protected:

	std::vector<char *> serviceNameArray;
	std::vector<ClientServerInfo*> clientServerVector;
	AtmiBroker::ClientCallback_var clientCallback;
	CORBA::String_var clientCallbackIOR;
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

extern ATMIBROKER_DLL AtmiBroker::ClientInfo clientInfo;

#endif
