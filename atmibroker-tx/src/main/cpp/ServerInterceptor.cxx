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
#include "ServerInterceptor.h"

#include "ace/Thread_Manager.h"
#include "tao/PI_Server/ServerRequestInfoA.h"
#include "tao/OctetSeqC.h"

#include <iostream>

LoggerPtr atmiServerInterceptorLogger(Logger::getLogger("ServerInterceptor"));

ServerInterceptor::ServerInterceptor(const char *orbname, IOP::CodecFactory_var cf) :
	TxInterceptor(orbname, cf, "ATMIServerTxInterceptor") { }

void ServerInterceptor::receive_request(PortableInterceptor::ServerRequestInfo_ptr ri) {
	debug(ri, ":receive_request");
	this->update_tx_context(ri);
}
void ServerInterceptor::receive_request_service_contexts(PortableInterceptor::ServerRequestInfo_ptr ri) {
	debug(ri, ":receive_request_service_contexts");
}
void ServerInterceptor::send_reply(PortableInterceptor::ServerRequestInfo_ptr ri) {
	debug(ri, ":send_reply");
}
void ServerInterceptor::send_exception(PortableInterceptor::ServerRequestInfo_ptr ri) {
	debug(ri, ":send_exception");
}
void ServerInterceptor::send_other(PortableInterceptor::ServerRequestInfo_ptr ri) {
	debug(ri, ":send_other");
}

bool ServerInterceptor::policyCheck(PortableInterceptor::ServerRequestInfo_ptr ri) {
        try {
                CORBA::Policy_var pv =  ri->get_server_policy(AtmiTx::OTS_POLICY_TYPE);
                AtmiTx::OTSPolicy_var otsp = AtmiTx::OTSPolicy::_narrow(pv.in());
                LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(), (char*) "policy var " << otsp);

                if (CORBA::is_nil(otsp.in())) {
                        LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(), (char*) "\tpolicy is nil");
                } else {
                        tpv_ = otsp->tpv();
                        LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(),
                                (char*) "\tpolicy value: " << tpv_);

                        /*
			 * if there is a transaction already associated with this thread and the policy is forbids
			 * or if the policy is requires but no transaction is associated then throw and (application)
			 * exception
			 */
			bool hastx = getSpecific(TSS_KEY) != 0;

                        if ((tpv_ == AtmiTx::FORBIDS && hastx) ||
                            (tpv_ == AtmiTx::REQUIRES && !hastx))
                        {
                                CORBA::TRANSACTION_MODE ex;
                                throw ex;
                        }
                }

                return true;
        } catch(const CORBA::BAD_PARAM& ex) {
                ex._tao_print_exception("ServerInterceptor policy error: ");
        } catch (const CORBA::SystemException& ex) {
                ex._tao_print_exception("ServerInterceptor policy error: ");
        } catch( ... ) {
                LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getWarn(), (char*) "GENERIC POLICY ERROR ...");
	}

	return false;
}
