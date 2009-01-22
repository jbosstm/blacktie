/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat Middleware LLC, and others contributors as indicated
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

#include "AtmiBrokerOTS.h"
#include "TxPolicyC.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;

// ORB request service context id for tagging service contexts (as transactional)
const CORBA::ULong tx_context_id = 0xabcd;

/**
 * Base class for orb request intercpetors for implicit transaction propagation
 */
class ATMIBROKER_TX_DLL TxInterceptor
{
public:
        TxInterceptor(CORBA::ORB_ptr*, IOP::CodecFactory_var, const char*);
        virtual ~TxInterceptor() {}

        virtual char *name();
        virtual void destroy() {}

	CORBA::ORB_ptr get_orb() { return *orb; }
protected:
        char* get_control_ior();
	CosTransactions::Control_ptr string_to_control(const char* ior);
	CosTransactions::Control_ptr current_control();
	bool isTransactional(const char*);

        const char *name_;
        IOP::Codec_var codec_;
        AtmiTx::OTSPolicyValue tpv_;
	CORBA::ORB_ptr* orb;

private:
};

#endif
