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

#ifndef AtmiBroker_ENV_XML_H_
#define AtmiBroker_ENV_XML_H_

#include "atmiBrokerCoreMacro.h"

#include <string.h>

#include <vector>

typedef struct {
	char * name;
	char * value;
} envVar_t;

class ATMIBROKER_CORE_DLL AtmiBrokerEnvXml {
public:

	AtmiBrokerEnvXml();

	~AtmiBrokerEnvXml();

	bool parseXmlDescriptor(std::vector<envVar_t>*, const char * aDescriptorFileName);

};

extern ATMIBROKER_CORE_DLL char* company;
extern ATMIBROKER_CORE_DLL char domain[30];
extern ATMIBROKER_CORE_DLL char server[30];
extern ATMIBROKER_CORE_DLL char * loggingServiceId;
extern ATMIBROKER_CORE_DLL char * notifyServiceId;
extern ATMIBROKER_CORE_DLL char * namingServiceId;
extern ATMIBROKER_CORE_DLL char * queue_name;
extern ATMIBROKER_CORE_DLL char* transFactoryId;
extern ATMIBROKER_CORE_DLL char* xaResourceMgrId;
extern ATMIBROKER_CORE_DLL char* xaResourceName;
extern ATMIBROKER_CORE_DLL char* xaOpenString;
extern ATMIBROKER_CORE_DLL char* xaCloseString;
extern ATMIBROKER_CORE_DLL bool xaThreadModel;
extern ATMIBROKER_CORE_DLL bool xaAutomaticAssociation;
extern ATMIBROKER_CORE_DLL bool xaDynamicRegistrationOptimization;

#endif
