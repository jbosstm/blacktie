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

#ifndef EndpointQueue_H_
#define EndpointQueue_H_

#include "atmiBrokerStompMacro.h"

#include <queue>

#ifdef __cplusplus
extern "C"
{
#endif
#include "stomp.h"
#ifdef __cplusplus
}
#endif

#include "log4cxx/logger.h"
#include "Destination.h"
#include "SynchronizableObject.h"

class BLACKTIE_STOMP_DLL EndpointQueue: public virtual Destination {
public:
	EndpointQueue(stomp_connection* connection, apr_pool_t* pool, char* serviceName);
	EndpointQueue(stomp_connection* connection, apr_pool_t* pool, char* serviceName, int id);
	virtual ~EndpointQueue();

	virtual void disconnect();

	virtual MESSAGE receive(long time);

	virtual const char* getName();
	const char* getFullName();
private:
	static log4cxx::LoggerPtr logger;
	stomp_connection* connection;
	apr_pool_t* pool;
	SynchronizableObject* lock;
	bool shutdown;
	char* name;
	const char* fullName;
};

#endif
