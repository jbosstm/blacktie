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

CosTransactions::Control_ptr ServerInterceptor::get_tx(PortableInterceptor::ServerRequestInfo_ptr ri, const char *msg)
{
	if (!isTransactional(ri)) {
		LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(),
			this->name_ << msg << (char*) " get_tx: " <<  ri->operation () << (char*) " not transactional");
		return NULL;
	}

	// extract the transaction context from the request
	IOP::ServiceContext_var sc = ri->get_request_service_context(tx_context_id);
	CORBA::OctetSeq ocSeq = sc->context_data;
	const char * ior = reinterpret_cast<const char *> (ocSeq.get_buffer ());
	CosTransactions::Control_ptr cptr = this->string_to_control(ior);

	LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(),
		this->name_ << msg << (char*) " get_tx: " <<  ri->operation () << (char*) " ior: " << ior << " control: " << cptr);

	return cptr;
}

ServerInterceptor::ServerInterceptor(CORBA::ORB_ptr* orbPtr, IOP::CodecFactory_var cf)
	: TxInterceptor(orbPtr, cf, "ATMIServerTxInterceptor")
{
}

void ServerInterceptor::receive_request_service_contexts(PortableInterceptor::ServerRequestInfo_ptr ri)
{
	//(void) get_tx(ri, ":receive_request_service_contexts");
}

void ServerInterceptor::receive_request(PortableInterceptor::ServerRequestInfo_ptr ri)
{
	CosTransactions::Control_ptr p = get_tx(ri, ":receive_request");

	if (!CORBA::is_nil(p)) {
		CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

		if (!CORBA::is_nil(curr) && !curr->_is_equivalent(p)) {
			// TODO ? should we suspend c and resume it in send_reply/send_exception
			LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getWarn(),
				(char*) "\tTODO current and context are different");

			CORBA::release(p);
		} else {
			LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(),
				(char*) "\ttransactional: " << p);
			// AtmiBroker_ServiceImpl::createConnectionTransactionAssociation calls getSpecific to retrieve the tx
			setSpecific(TSS_KEY, p);
		}
	}
}

void ServerInterceptor::send_reply(PortableInterceptor::ServerRequestInfo_ptr ri)
{
	CosTransactions::Control_ptr p = get_tx(ri, ":send_reply");

	if (!CORBA::is_nil(p)) {
		CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

		LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(), (char*) "\tcurr=" << curr);

		if (!CORBA::is_nil(curr)) {
			LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getDebug(),
				(char*) "\t?? should we suspend current?");
			destroySpecific(TSS_KEY);
		}

		CORBA::release(p);
	}
}

void ServerInterceptor::send_exception(PortableInterceptor::ServerRequestInfo_ptr ri)
{
	//(void) get_tx(ri, ":send_exception");
	// TODO figure out the correct semantics
	// for now just duplicate what we do in send_reply
	send_reply(ri);
}

void ServerInterceptor::send_other(PortableInterceptor::ServerRequestInfo_ptr ri)
{
}

/**
 * see if the inbound IOR contains an OTS policy tag
 */
bool ServerInterceptor::isTransactional(PortableInterceptor::ServerRequestInfo_ptr ri)
{
	if (!this->TxInterceptor::isTransactional((const char*) ri->operation ()))
		return false;

	try {
		CORBA::Policy_var pv =  ri->get_server_policy(AtmiTx::OTS_POLICY_TYPE);
		return true;
	} catch (const CORBA::SystemException& ex) {
		// should be INV_POLICY, minor code 2 
		//ex._tao_print_exception("requested object is not transactional:");
	} catch (...) {
		LOG4CXX_LOGLS(atmiServerInterceptorLogger, Level::getWarn(),
			(char*) "isTransactional unexpected ex, op: " << ri->operation ());
	}

	return false;
}

/**
 * If a request contains a transactional policy make sure that the target thread matches that policy
 */
bool ServerInterceptor::policyCheck(PortableInterceptor::ServerRequestInfo_ptr ri)
{
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
		//ex._tao_print_exception("ServerInterceptor policy error: ");
	}

	return false;
}
