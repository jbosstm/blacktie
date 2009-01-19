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

extern bool initialized;
extern bool checkForInitialized();

extern ATMIBROKER_DLL int _tperrno;
extern ATMIBROKER_DLL long _tpurcode;

// Constants
extern int MAX_SERVERS;
extern int MAX_SERVICES;
extern int MAX_SERVICE_CACHE_SIZE;
extern int MIN_SERVICE_CACHE_SIZE;
extern int MIN_AVAILABLE_SERVICE_CACHE_SIZE;
extern int MAX_COMPANY_SIZE;
extern const char * SERVER_LIT;
extern const char * MAX_REPLICAS;
extern const char * MIN;
extern const char * MAX;
extern const char * AVAILABLE;

#endif //AtmiBroker_H
