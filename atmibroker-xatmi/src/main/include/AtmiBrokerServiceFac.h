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

// AtmiBrokerServiceFac.h

#ifndef AtmiBroker_SERVICE_FAC_H
#define AtmiBroker_SERVICE_FAC_H

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "AtmiBrokerS.h"
#elif ORBIX_COMP
#include "AtmiBroker.hh"
#endif
#ifdef VBC_COMP
#include "AtmiBroker_s.hh"
#endif

#include "AtmiBroker_ServiceFactoryImpl.h"
#include "AtmiBroker_ServiceManagerImpl.h"

extern ATMIBROKER_DLL void remove_service_factory(char * serviceName);
extern ATMIBROKER_DLL void remove_service_manager(char * serviceName);

extern ATMIBROKER_DLL PortableServer::POA_ptr create_service_factory_poa(char *serviceName);
extern ATMIBROKER_DLL PortableServer::POA_ptr create_service_manager_poa(char *serviceName);

extern ATMIBROKER_DLL void create_service_factory(AtmiBroker_ServiceFactoryImpl *tmp_servant, PortableServer::POA_var aPoaPtr, char *serviceName, void(*func)(TPSVCINFO *));
extern ATMIBROKER_DLL void create_service_manager(AtmiBroker_ServiceManagerImpl *tmp_servant, PortableServer::POA_var aPoaPtr, char *serviceName);

#endif //AtmiBroker_SERVICE_FAC_H
