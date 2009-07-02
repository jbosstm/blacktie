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
#ifndef SessionImpl_H_
#define SessionImpl_H_

#include "atmiBrokerHybridMacro.h"

#include "log4cxx/logger.h"
#include "Session.h"
#include "ConnectionImpl.h"
#include "CorbaConnection.h"
#include "CorbaEndpointQueue.h"
#include "StompEndpointQueue.h"

class HybridConnectionImpl;

class BLACKTIE_HYBRID_DLL HybridSessionImpl: public virtual Session {
public:
	HybridSessionImpl(CORBA_CONNECTION* connection, int id, const char* temporaryQueueName);

	HybridSessionImpl(CORBA_CONNECTION* connection, apr_pool_t* pool, int id, char* service);

	virtual ~HybridSessionImpl();

	void setSendTo(const char* replyTo);

	const char* getReplyTo();

	MESSAGE receive(long time);

	bool send(MESSAGE message);

	int getId();
private:
	static log4cxx::LoggerPtr logger;
	int id;
	CORBA_CONNECTION* corbaConnection;
	CorbaEndpointQueue* temporaryQueue;
	AtmiBroker::EndpointQueue_var remoteEndpoint;
	stomp_connection* stompConnection;
	apr_pool_t* pool;
	const char* replyTo;
	char* sendTo;
	bool serviceInvokation;
};

#endif
