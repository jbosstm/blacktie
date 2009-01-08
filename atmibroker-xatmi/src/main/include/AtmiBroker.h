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

// AtmiBroker.h

#ifndef AtmiBroker_H
#define AtmiBroker_H

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>
#include <tao/PortableServer/PortableServerC.h>
#elif ORBIX_COMP
#include <omg/orb.hh>
#include <omg/CosNaming.hh>
#include <omg/PortableServer.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#include <CosNaming_c.hh>
#include <PortableServerExt_c.hh>
#endif

extern "C" {
#include "xatmi.h"
}

#include <time.h>

#include "AtmiBrokerServerFac.h"

extern bool initialized;
extern bool checkForInitialized();

extern ATMIBROKER_DLL int _tperrno;
extern ATMIBROKER_DLL long _tpurcode;

extern ATMIBROKER_DLL AtmiBroker::ClientInfo clientInfo;

// Constants
//TODO REPLACED WITH XATMI EQUIVALENT extern ATMIBROKER_DLL int						SERVICE_ID_SIZE;
extern ATMIBROKER_DLL int MAX_SERVERS;
extern ATMIBROKER_DLL int MAX_SERVICES;
extern ATMIBROKER_DLL int MAX_SERVICE_CACHE_SIZE;
extern ATMIBROKER_DLL int MIN_SERVICE_CACHE_SIZE;
extern ATMIBROKER_DLL int MIN_AVAILABLE_SERVICE_CACHE_SIZE;
extern ATMIBROKER_DLL int MAX_COMPANY_SIZE;
extern ATMIBROKER_DLL const char * SERVER_LIT;
extern ATMIBROKER_DLL const char * MAX_REPLICAS;
extern ATMIBROKER_DLL const char * MIN;
extern ATMIBROKER_DLL const char * MAX;
extern ATMIBROKER_DLL const char * AVAILABLE;

extern ATMIBROKER_DLL void getRootPOAAndManager(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager);
extern ATMIBROKER_DLL void termination_handler_callback(long);
extern ATMIBROKER_DLL void client_termination_handler_callback(int);
extern ATMIBROKER_DLL void server_termination_handler_callback(int);

#endif //AtmiBroker_H
