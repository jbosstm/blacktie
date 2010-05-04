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
#ifndef HybridConnectionImpl_H_
#define HybridConnectionImpl_H_

#include "atmiBrokerHybridMacro.h"

#include <map>

#ifdef __cplusplus
extern "C" {
#endif
#include "stomp.h"
#ifdef __cplusplus
}
#endif

#include "log4cxx/logger.h"
#include "CorbaConnection.h"
#include "Connection.h"
#include "Destination.h"
#include "HybridSessionImpl.h"

class HybridSessionImpl;

class BLACKTIE_HYBRID_DLL HybridConnectionImpl: public virtual Connection {
public:
	HybridConnectionImpl(char* connectionName, void(*messagesAvailableCallback)(int,bool));
	virtual ~HybridConnectionImpl();

	Session* createSession(int id, char* serviceName);
	Session* createSession(int id, const char* temporaryQueueName);
	Session* getSession(int id);
	void closeSession(int id);
	void disconnectSession(int id);

	static stomp_connection* connect(apr_pool_t* pool, int timeout);
	static void disconnect(stomp_connection* connection, apr_pool_t* pool);

	Destination* createDestination(char* serviceName);
	void destroyDestination(Destination* destination);

private:
	static log4cxx::LoggerPtr logger;
	char* connectionName;
	std::map<int, HybridSessionImpl*> sessionMap;
	apr_pool_t* pool;
	CORBA_CONNECTION* connection;
	void(*messagesAvailableCallback)(int,bool);
};

#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_HYBRID_DLL struct connection_factory_t connectionFactory;
#ifdef __cplusplus
}
#endif

#endif
