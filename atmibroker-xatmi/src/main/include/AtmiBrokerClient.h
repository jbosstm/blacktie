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


#ifndef AtmiBroker_CLIENT_H_
#define AtmiBroker_CLIENT_H_

#include <vector>

#include "Connection.h"
#include "Session.h"
#include "AtmiBrokerClientXml.h"

class AtmiBrokerClient {
public:
	AtmiBrokerClient();
	virtual ~AtmiBrokerClient();
	Session* createSession(int& id, char* serviceName);
	Session* getSession(int id);
	void closeSession(int id);
protected:
	Connection* clientConnection;
	std::vector<ClientServerInfo*> clientServerVector;
	int nextSessionId;
};

// CLIENT
extern AtmiBrokerClient* ptrAtmiBrokerClient;

extern char* TSS_TPERESET;
extern char* TSS_TPEBADDESC;
extern char* TSS_TPEBLOCK;
extern char* TSS_TPEINVAL;
extern char* TSS_TPELIMIT;
extern char* TSS_TPENOENT;
extern char* TSS_TPEOS;
extern char* TSS_TPEPROTO;
extern char* TSS_TPESVCERR;
extern char* TSS_TPESVCFAIL;
extern char* TSS_TPESYSTEM;
extern char* TSS_TPETIME;
extern char* TSS_TPETRAN;
extern char* TSS_TPGOTSIG;
extern char* TSS_TPEITYPE;
extern char* TSS_TPEOTYPE;
extern char* TSS_TPEEVENT;
extern char* TSS_TPEMATCH;

#endif
