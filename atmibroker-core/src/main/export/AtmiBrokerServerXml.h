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

#ifndef AtmiBroker_SERVER_XML_H_
#define AtmiBroker_SERVER_XML_H_

#include "atmiBrokerCoreMacro.h"
#include <string.h>
#include <vector>

struct ServerMetadata {
	short maxChannels;
	short maxSuppliers;
	short maxConsumers;
	short maxReplicas;
	short logLevel;
	std::string securityType;
	std::string orbType;
	std::string queueSpaceName;
	std::vector<std::string> serviceNames;
};

class BLACKTIE_CORE_DLL AtmiBrokerServerXml {
public:

	AtmiBrokerServerXml();

	~AtmiBrokerServerXml();

	bool parseXmlDescriptor(ServerMetadata*, const char * aDescriptorFileName);
};

extern BLACKTIE_CORE_DLL char server[30];
extern BLACKTIE_CORE_DLL int serverMaxChannels;
extern BLACKTIE_CORE_DLL int serverMaxSuppliers;
extern BLACKTIE_CORE_DLL int serverMaxConsumers;

#endif
