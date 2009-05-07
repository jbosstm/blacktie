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
#ifndef ConnectionImpl_H_
#define ConnectionImpl_H_

#include "atmiBrokerStompMacro.h"

#include <map>

#ifdef __cplusplus
extern "C" 
{
#endif
#include "stomp.h"
#ifdef __cplusplus
}
#endif

#include "log4cxx/logger.h"
#include "Connection.h"
#include "Destination.h"
#include "SessionImpl.h"

class SessionImpl;

class BLACKTIE_STOMP_DLL ConnectionImpl: public virtual Connection {
public:
	ConnectionImpl(char* connectionName);
	virtual ~ConnectionImpl();

	Session* createSession(int id, char* serviceName);
	Session* createSession(int id, const char* temporaryQueueName);
	Session* getSession(int id);
	void closeSession(int id);

	static stomp_connection* connect(apr_pool_t* pool, int timeout);
	static void disconnect(stomp_connection* connection, apr_pool_t* pool);

	Destination* createDestination(char* serviceName);
	void destroyDestination(Destination* destination);
private:
	static log4cxx::LoggerPtr logger;
	char* connectionName;
	std::map<int, SessionImpl*> sessionMap;
	apr_pool_t* pool;
	stomp_connection* connection;
};

#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_STOMP_DLL struct connection_factory_t connectionFactory;
#ifdef __cplusplus
}
#endif

#endif
