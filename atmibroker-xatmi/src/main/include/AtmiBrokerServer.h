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
// Edit the idlgen.cfg to have your own copyright notice placed here.
//-----------------------------------------------------------------------------

// Class: AtmiBroker_ServerImpl
// A POA servant which implements of the AtmiBroker::Server interface
//


#ifndef ATMIBROKER_SERVERSERVER_H_
#define ATMIBROKER_SERVERSERVER_H_

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

#include "AtmiBrokerPoaFac.h"
#include "AtmiBroker_ServerImpl.h"
#include "Worker.h"

extern Worker* server_worker;
extern CORBA::ORB_var server_orb;
extern PortableServer::POA_var server_root_poa;
extern PortableServer::POAManager_var server_root_poa_manager;
extern CosNaming::NamingContextExt_var server_default_context;
extern CosNaming::NamingContext_var server_name_context;
extern PortableServer::POA_var server_poa;
extern PortableServer::POA_var server_callback_poa;
extern AtmiBrokerPoaFac * serverPoaFactory;

extern AtmiBroker_ServerImpl * ptrServer;

#endif
