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
#ifndef OrbManagement_H
#define OrbManagement_H

#include "atmiBrokerCoreMacro.h"

#include "Worker.h"

#ifdef TAO_COMP
#include <tao/ORB.h>
#include <orbsvcs/CosNamingS.h>
#include <tao/PortableServer/PortableServerC.h>
#endif

extern ATMIBROKER_CORE_DLL void initOrb(char* name, Worker*& worker, CORBA::ORB_ptr& orbRef);
extern ATMIBROKER_CORE_DLL void getNamingServiceAndContext(CORBA::ORB_ptr& orbRef, CosNaming::NamingContextExt_var& default_ctx, CosNaming::NamingContext_var& name_ctx);
extern ATMIBROKER_CORE_DLL void getRootPOAAndManager(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager);
extern ATMIBROKER_CORE_DLL void shutdownBindings(CORBA::ORB_ptr& orbRef, PortableServer::POA_var& poa, PortableServer::POAManager_var& poa_manager, CosNaming::NamingContextExt_var& ctx, CosNaming::NamingContext_var& nameCtx, PortableServer::POA_var& innerPoa, Worker* worker);

#endif
