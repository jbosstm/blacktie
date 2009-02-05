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

#include "log4cxx/logger.h"
#include "Sender.h"
#include "Destination.h"

class SenderImpl: public virtual Sender {
public:
	SenderImpl(void* orb, char * callback_ior);
	SenderImpl(void* connection_context, void* connection_name_context, const char * serviceName);
	virtual ~SenderImpl();
	virtual void send(MESSAGE message);
	virtual void disconnect();
private:
	static log4cxx::LoggerPtr logger;
	Destination* destination;
};

#endif
