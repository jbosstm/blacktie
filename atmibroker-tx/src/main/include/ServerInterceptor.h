/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#ifndef SERVERINTERCEPTOR_H
#define SERVERINTERCEPTOR_H

#include "tao/PortableInterceptorC.h"
#include "tao/PI_Server/ServerRequestInterceptorA.h"

#include "TxInterceptor.h"

/**
 * Server side orb request intercpetor for implicit transaction propagation
 */
class BLACKTIE_TX_DLL ServerInterceptor:
	public virtual TxInterceptor,
	public PortableInterceptor::ServerRequestInterceptor
{
public:
	ServerInterceptor(const char *, IOP::CodecFactory_var);
	virtual ~ServerInterceptor() {}

	virtual char* name() { return TxInterceptor::name();}
	virtual void destroy() { TxInterceptor::destroy();}

	virtual void receive_request(PortableInterceptor::ServerRequestInfo_ptr);
	virtual void receive_request_service_contexts(PortableInterceptor::ServerRequestInfo_ptr);
	virtual void send_reply(PortableInterceptor::ServerRequestInfo_ptr);
	virtual void send_exception(PortableInterceptor::ServerRequestInfo_ptr);
	virtual void send_other(PortableInterceptor::ServerRequestInfo_ptr);

private:
	bool policyCheck(PortableInterceptor::ServerRequestInfo_ptr);
};

#endif
