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

#ifndef AtmiBroker_CLIENT_XML_H_
#define AtmiBroker_CLIENT_XML_H_

#include "atmiBrokerMacro.h"

#ifdef TAO_COMP
#include "tao/ORB.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif

#include <iostream>
#include <string.h>

#include "AtmiBrokerClient.h"
#include <vector>

class ATMIBROKER_DLL AtmiBrokerClientXml {
public:

	static const char* Client_Begin_Tag;
	static const char* Client_End_Tag;

	static const char* Client_Desc_Begin_Tag;
	static const char* Client_Desc_End_Tag;

	static const char* Max_Replicas_Begin_Tag;
	static const char* Max_Replicas_End_Tag;

	static const char* Max_Channels_Begin_Tag;
	static const char* Max_Channels_End_Tag;

	static const char* Max_Suppliers_Begin_Tag;
	static const char* Max_Suppliers_End_Tag;

	static const char* Max_Consumers_Begin_Tag;
	static const char* Max_Consumers_End_Tag;

	static const char* Server_Names_Begin_Tag;
	static const char* Server_Names_End_Tag;

	static const char* Server_Name_Begin_Tag;
	static const char* Server_Name_End_Tag;

	static const char* Service_Names_Begin_Tag;
	static const char* Service_Names_End_Tag;

	static const char* Service_Name_Begin_Tag;
	static const char* Service_Name_End_Tag;

	AtmiBrokerClientXml();

	~AtmiBrokerClientXml();

	const char * writeXmlDescriptor(std::vector<ClientServerInfo*>* serverData);

	void parseXmlDescriptor(std::vector<ClientServerInfo*>*, const char * aDescriptorFileName);
};

extern ATMIBROKER_DLL int clientMaxChannels;
extern ATMIBROKER_DLL int clientMaxSuppliers;
extern ATMIBROKER_DLL int clientMaxConsumers;

#endif
