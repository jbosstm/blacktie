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

// AtmiBrokerConversation.h

#ifndef AtmiBroker_CONVERSATION_H
#define AtmiBroker_CONVERSATION_H

#include "atmiBrokerMacro.h"

#include "AtmiBrokerClient.h"

#include <iostream>
#include <vector>

class ATMIBROKER_DLL AtmiBrokerConversation {

public:

	AtmiBrokerConversation(AtmiBrokerClient* aAtmiBrokerClient);

	~AtmiBrokerConversation();

	char* getenv(char* anEnvName);

	int tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen, long flags);

	int tpacall(char * svc, char* idata, long ilen, long flags);

	int tpconnect(char * svc, char* idata, long ilen, long flags);

	int tpsend(int id, char* idata, long ilen, long flags, long *revent);

	int tprecv(int id, char ** odata, long *olen, long flags, long* event);

	int tpgetrply(int *id, char ** odata, long *olen, long flags);

	int tpdiscon(int id);

	int tpcancel(int id);

	static AtmiBrokerConversation* get_instance();
	static void discard_instance();

private:
	int send(AtmiBroker::Service_var, char* idata, long ilen, bool inConversation, long flags, long *revent);
	int end(int id);

	AtmiBrokerClient* mAtmiBrokerClient;

	static AtmiBrokerConversation* ptrAtmiBrokerConversation;
};

#endif //AtmiBroker_CONVERSATION_H
