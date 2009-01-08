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

#ifndef AtmiBroker_SERVICE_XML_H_
#define AtmiBroker_SERVICE_XML_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "tao/ORB.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif

#include <string.h>

#include "AtmiBroker_ServiceFactoryImpl.h"

class ATMIBROKER_DLL AtmiBrokerServiceXml {
public:

	static const char* Service_Begin_Tag;
	static const char* Service_End_Tag;

	static const char* Service_Desc_Begin_Tag;
	static const char* Service_Desc_End_Tag;

	static const char* Max_Cache_Begin_Tag;
	static const char* Max_Cache_End_Tag;

	static const char* Min_Cache_Begin_Tag;
	static const char* Min_Cache_End_Tag;

	static const char* Min_Available_Cache_Begin_Tag;
	static const char* Min_Available_Cache_End_Tag;

	AtmiBrokerServiceXml();

	~AtmiBrokerServiceXml();

	const char * writeXmlDescriptor(AtmiBroker::ServiceInfo* serviceData);

	void parseXmlDescriptor(AtmiBroker::ServiceInfo* serviceData, const char * aDescriptorFileName);
};

#endif
