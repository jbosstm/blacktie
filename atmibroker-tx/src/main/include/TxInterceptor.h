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
#ifndef TX_INTERCEPTOR_H
#define TX_INTERCEPTOR_H

#include "atmiBrokerTxMacro.h"
#include "ThreadLocalStorage.h"

#include "tao/PI/ORBInitInfo.h"
#include "tao/PI/ClientRequestInfo.h"
#include "tao/PI_Server/ServerRequestInfoA.h"

#include "AtmiBrokerOTS.h"
#include "TxPolicyC.h"

#include "log4cxx/logger.h"



// ORB request service context id for tagging service contexts (as transactional)
const CORBA::ULong tx_context_id = 0xabcd;

/**
 * Base class for orb request intercpetors for implicit transaction propagation
 */
class ATMIBROKER_TX_DLL TxInterceptor
{
public:
        TxInterceptor(const char *, IOP::CodecFactory_var, const char*);
        virtual ~TxInterceptor();

        virtual char *name();
        virtual void destroy() {}

	const char * get_orb() { return orbname_; }
protected:
        char* get_control_ior();
	CosTransactions::Control_ptr current_control();

	bool isTransactional(PortableInterceptor::ServerRequestInfo_ptr);
	bool isTransactional(PortableInterceptor::ClientRequestInfo_ptr);
	char * extract_ior_from_context(PortableInterceptor::ClientRequestInfo_ptr);
	char * extract_ior_from_context(PortableInterceptor::ServerRequestInfo_ptr);
	CosTransactions::Control_ptr extract_tx_from_context(PortableInterceptor::ClientRequestInfo_ptr);
	CosTransactions::Control_ptr extract_tx_from_context(PortableInterceptor::ServerRequestInfo_ptr);
	bool add_ior_to_context(PortableInterceptor::ClientRequestInfo_ptr);
	CosTransactions::Control_ptr ior_to_control(char *);
	void update_tx_context(PortableInterceptor::ServerRequestInfo_ptr);
	void debug(bool transactional, const char * msg, const char* op);
	void debug(PortableInterceptor::ClientRequestInfo_ptr, const char* msg);
	void debug(PortableInterceptor::ServerRequestInfo_ptr, const char* msg);

        const char *name_;
        IOP::Codec_var codec_;
        AtmiTx::OTSPolicyValue tpv_;
	char * orbname_;

private:
};

#endif
