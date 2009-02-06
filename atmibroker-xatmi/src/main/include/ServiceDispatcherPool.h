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

// Class: ServiceDispatcherPool
// A POA servant which implements of the AtmiBroker::ServiceFactory interface
//

#ifndef ServiceDispatcherPool_H_
#define ServiceDispatcherPool_H_

#include "atmiBrokerMacro.h"

#include <vector>
#include "log4cxx/logger.h"
#include "xatmi.h"
#include "ServiceDispatcher.h"
#include "Connection.h"
#include "Destination.h"
#include "AtmiBrokerServiceXml.h"

class ATMIBROKER_DLL ServiceDispatcherPool {
public:
	ServiceDispatcherPool(CONNECTION* connection, Destination* destination, char *serviceName, void(*func)(TPSVCINFO *));
	virtual ~ServiceDispatcherPool();
	SVCINFO get_service_info();
	Destination* getDestination();
private:
	static log4cxx::LoggerPtr logger;
	Destination* destination;
	char* serviceName;
	std::vector<ServiceDispatcher *> dispatchers;
	SVCINFO serviceInfo;
	// The following are not implemented
	//
	ServiceDispatcherPool(const ServiceDispatcherPool &);
	ServiceDispatcherPool& operator=(const ServiceDispatcherPool &);
};

#endif
