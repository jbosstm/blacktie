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
#ifndef SenderImpl_H_
#define SenderImpl_H_

#include "atmiBrokerCorbaMacro.h"

#include "log4cxx/logger.h"
#include "Sender.h"
#include "Destination.h"
#include "EndpointQueue.h"

class ATMIBROKER_CORBA_DLL SenderImpl: public virtual Sender {
public:
	SenderImpl(char* name, AtmiBroker::EndpointQueue_ptr destination);
	virtual ~SenderImpl();
	virtual void send(MESSAGE message);
	virtual void close();
private:
	static log4cxx::LoggerPtr logger;
	char* name;
	AtmiBroker::EndpointQueue_ptr destination;
};

#endif
