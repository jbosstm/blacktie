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

#include <map>

#include "atmiBrokerCorbaMacro.h"
#include "log4cxx/logger.h"
#include "CorbaConnection.h"
#include "Connection.h"
#include "Destination.h"
#include "SessionImpl.h"

class CorbaSessionImpl;

class BLACKTIE_CORBA_DLL CorbaConnectionImpl: public virtual Connection {
public:
	CorbaConnectionImpl(char* connectionName);
	virtual ~CorbaConnectionImpl();

	Session* createSession(int id, char* serviceName);
	Session* createSession(int id, const char* temporaryQueueName);
	Session* getSession(int id);
	void closeSession(int id);

	Destination* createDestination(char* serviceName);
	void destroyDestination(Destination* destination);
private:
	static log4cxx::LoggerPtr logger;
	CORBA_CONNECTION* connection;
	std::map<int, CorbaSessionImpl*> sessionMap;
};

#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_CORBA_DLL struct connection_factory_t connectionFactory;
#ifdef __cplusplus
}
#endif

#endif
