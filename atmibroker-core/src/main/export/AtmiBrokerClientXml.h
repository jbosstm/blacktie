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

#ifndef AtmiBroker_CLIENT_XML_H_
#define AtmiBroker_CLIENT_XML_H_

#include <vector>
#include "atmiBrokerCoreMacro.h"

typedef struct _client_service_info {
	char* serviceName;
	char* transportLib;
} ClientServiceInfo;

struct _client_server_info {
	char * serverName;
	std::vector<ClientServiceInfo> serviceVector;
};
typedef _client_server_info ClientServerInfo;

class BLACKTIE_CORE_DLL AtmiBrokerClientXml {
public:
	AtmiBrokerClientXml();

	~AtmiBrokerClientXml();

	bool parseXmlDescriptor(std::vector<ClientServerInfo*>*, const char * aDescriptorFileName);
};

extern BLACKTIE_CORE_DLL int clientMaxChannels;
extern BLACKTIE_CORE_DLL int clientMaxSuppliers;
extern BLACKTIE_CORE_DLL int clientMaxConsumers;

#endif
