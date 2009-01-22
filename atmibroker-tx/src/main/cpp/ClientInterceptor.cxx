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
#include "ClientInterceptor.h"

#include "tao/OctetSeqC.h"
#include "tao/LocalObject.h"
#include "tao/PI/ClientRequestInfo.h"

#include <iostream>

LoggerPtr atmiClientInterceptorlogger(Logger::getLogger("ClientInterceptor"));

void ClientInterceptor::debug(PortableInterceptor::ClientRequestInfo_ptr ri, const char* msg)
{
	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
		this->name_ << msg << (char*) " " <<  ri->operation () << (char*) " transactional=" << isTransactional(ri));
}

ClientInterceptor::ClientInterceptor(CORBA::ORB_ptr* orbPtr, IOP::CodecFactory_var cf) : TxInterceptor(orbPtr, cf, "ATMIClientTxInterceptor")
{
}

void ClientInterceptor::send_poll(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	//debug(ri, ":send_poll");
}

void
ClientInterceptor::send_request(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	//debug(ri, ":send_request");
	if (!isTransactional(ri))
		 return;

	char* ior = this->get_control_ior();

	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
		 (char*) "send_request: op: " << ri->operation () << (char*) " ior: " << ior);

	if (ior)
	{
		// suspend the current transaction
		try {
			long txid = 0L;

			AtmiBrokerOTS::get_instance()->suspend(txid);
			// and remember it so that it can be resumed when server replies
			CORBA::ULong rid = ri->request_id();
			pending[rid] = txid;	// what happens if the reply never comes - we'll leek memory

			LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
				(char *) "suspended " << rid << " <-> " << txid);

		} catch (CORBA::SystemException& ex) { // CosTransactions::InvalidControl
			// should be due to no active tx
			ex._tao_print_exception("TxInterceptor TAG_OTS_POLICY error: ");
		}

		// add the ior for the transaction to the service contexts
		IOP::ServiceContext sc;
		int slen = ACE_OS::strlen(ior) + 1;

		sc.context_id = tx_context_id;
		sc.context_data.length(slen);
		CORBA::Octet *sc_buf = sc.context_data.get_buffer();
		ACE_OS::memcpy(sc_buf, ior, slen);
		ri->add_request_service_context(sc, 1);

		CORBA::string_free(ior);
	 }
}

void
ClientInterceptor::receive_reply(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	debug(ri, ":receive_reply");
	if (!isTransactional(ri))
		 return;

	// Step 1: see if there is a transaction in the service contexts
	IOP::ServiceContext_var sc = ri->get_request_service_context(tx_context_id);
	CORBA::OctetSeq ocSeq = sc->context_data;
	const char * ior = reinterpret_cast<const char *> (ocSeq.get_buffer ());
	CosTransactions::Control_ptr cntx = this->string_to_control(ior);

	// Step 2: find out which transaction was active when the request was sent
	long txid = find_control(ri->request_id(), false);
        CosTransactions::Control_ptr orig = txid == 0L
		? CosTransactions::Control::_nil()
		: AtmiBrokerOTS::get_instance()->getSuspended(txid);

	// Step 3: is there already a transaction active on this thread
	CosTransactions::Control_ptr curr = this->current_control();

	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(), (char*) "receive_reply:"
		<< (char *) "\n\tcontext : " << cntx
		<< (char *) "\n\toriginal: " << orig
		<< (char *) "\n\tcurrent : " << curr);

	if (CORBA::is_nil(cntx)) {
		// no tx from server or server does not support implicit context propagation
		if (!CORBA::is_nil(orig))	// was there originally a tx on this request
			resume_tx(ri);	// resume the original tx
	} else if (!CORBA::is_nil(orig)) {
		// the tx from the server should match up with the tx on the original request:
		if (!cntx->_is_equivalent(orig)) {
			LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getWarn(),
				"receive_reply: server error: tx does not correspond with tx on original request");
		} else {	// req and resp transactions match so resume it:
			resume_tx(ri);	// resume the original tx
		}
	} else {
		LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getWarn(),
				"receive_reply: server sent a tx but original request was not transactional");
	}

	if (!CORBA::is_nil(cntx))
		CORBA::release(cntx);
}

void
ClientInterceptor::receive_other(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	debug(ri, ":receive_other");
	// probably a callback treat it as a normal reply
	receive_reply(ri);
}

void
ClientInterceptor::receive_exception(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
		this->name_ << ":receive_exception" << (char*) " " <<  ri->operation ());
	
	// TODO figure out what the correct semantics should be (rollback?)
	// minimally if the original request was transactional then resume it:
	if (isTransactional(ri))
		resume_tx(ri);
}

bool ClientInterceptor::isTransactional(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	if (!this->TxInterceptor::isTransactional((const char*) ri->operation ()))
		return false;

        try {
                IOP::TaggedComponent_var comp = ri->get_effective_component(AtmiTx::TAG_OTS_POLICY);
                return true;
        } catch (const CORBA::SystemException& ex) {
                // should be BAD_PARAM, minor code 25 
        } catch (...) {
                LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(), (char*) "isTransactional unexpected ex");
        }
        
        return false;
}

/**
 * lookup a transaction matching a given CORBA request id
 * if erase is true then the matching pair is removed from the pending request list
 */
long ClientInterceptor::find_control(CORBA::ULong requestId, bool erase)
{
	long id = pending[requestId];
        ActiveTxMap::iterator pos = pending.find(requestId);

        LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(), (char*) "find_control: found=" << (pos == pending.end()));

        if (pos == pending.end())
                return NULL;

	if (erase)
		pending.erase(pos);

        LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
		(char*) "find_control: " << pos->first << (char*) " <-> " << id);

	return id;
}

/**
 * resume a previously suspended transaction
 */
void ClientInterceptor::resume_tx(PortableInterceptor::ClientRequestInfo_ptr ri)
{
	long txid = find_control(ri->request_id(), true);

	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(), "resume_tx: txid=" << txid);

        CosTransactions::Control_ptr orig = (txid == 0L
		? CosTransactions::Control::_nil()
		: AtmiBrokerOTS::get_instance()->getSuspended(txid));
	CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) getSpecific(TSS_KEY);

	LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(),
		"resume_tx " << txid << " rid: " << ri->request_id() << "orig: " << orig << " curr: " << curr);

	if (!CORBA::is_nil(orig)) {
		if (CORBA::is_nil(curr)) {
			LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getDebug(), "curr=0, resuming transaction " << txid);
			setSpecific(TSS_KEY, orig);
			AtmiBrokerOTS::get_instance()->resume(txid);
		} else if (!curr->_is_equivalent(orig)) {
			LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getWarn(),
				(char*) "thread tx different from request tx -  ??? don't know how to nest transactions");
		} else {
			// must have received a reply on the same thread as the request;
			LOG4CXX_LOGLS(atmiClientInterceptorlogger, Level::getInfo(), "still same tx - resuming " << txid);
			AtmiBrokerOTS::get_instance()->resume(txid);
		}
	}
}
