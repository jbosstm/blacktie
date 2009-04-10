/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

#ifndef AtmiBroker_ENV_XML_H_
#define AtmiBroker_ENV_XML_H_

#include "atmiBrokerCoreMacro.h"

#include <string.h>

#include <vector>

typedef struct {
	char * name;
	char * value;
} envVar_t;

class BLACKTIE_CORE_DLL AtmiBrokerEnvXml {
public:

	AtmiBrokerEnvXml();

	~AtmiBrokerEnvXml();

	bool parseXmlDescriptor(std::vector<envVar_t>*, const char * aDescriptorFileName);

};

typedef struct BLACKTIE_CORE_DLL xarm_config {
	bool threadModel;
	bool automaticAssociation;
	bool dynamicRegistrationOptimization;
	long resourceMgrId;
	char * resourceName;
	char * openString;
	char * closeString;
	char * xasw;
	char * xalib;
	struct xarm_config * next;
	struct xarm_config * head;
}xarm_config_t;

extern BLACKTIE_CORE_DLL xarm_config_t * xarmp;
extern BLACKTIE_CORE_DLL char domain[30];
extern BLACKTIE_CORE_DLL char * loggingServiceId;
extern BLACKTIE_CORE_DLL char * notifyServiceId;
extern BLACKTIE_CORE_DLL char * namingServiceId;
extern BLACKTIE_CORE_DLL char * queue_name;
extern BLACKTIE_CORE_DLL char* transFactoryId;

#endif
