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
#ifndef CLIENTINTERCEPTOR_H
#define CLIENTINTERCEPTOR_H

#include "tao/PortableInterceptorC.h"
#include "tao/LocalObject.h"
#include "tao/PI/ClientRequestInterceptorA.h"

#include "TxInterceptor.h"

#include <map>

/**
 * Client side orb request intercpetor for implicit transaction propagation
 */
class BLACKTIE_TX_DLL ClientInterceptor:
        public virtual PortableInterceptor::ClientRequestInterceptor,
        public virtual ::CORBA::LocalObject,
        public virtual TxInterceptor
{
public:
        ClientInterceptor(const char *, IOP::CodecFactory_var);
        virtual ~ClientInterceptor() {}

        virtual char* name() { return TxInterceptor::name();}
        virtual void destroy() { TxInterceptor::destroy();}

        virtual void send_poll(PortableInterceptor::ClientRequestInfo_ptr ri);
        virtual void send_request(PortableInterceptor::ClientRequestInfo_ptr ri);
        virtual void receive_reply(PortableInterceptor::ClientRequestInfo_ptr ri);
        virtual void receive_other(PortableInterceptor::ClientRequestInfo_ptr ri);
        virtual void receive_exception(PortableInterceptor::ClientRequestInfo_ptr ri);

private:
        void resume_tx(PortableInterceptor::ClientRequestInfo_ptr);
	long find_control(CORBA::ULong requestId, bool);

        // outstanding requests to transaction map
        typedef std::map<CORBA::ULong, long> ActiveTxMap;

        ActiveTxMap pending;

private:
};

#endif // CLIENTINTERCEPTOR_H
