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
#include "TxInterceptor.h"

#include "ThreadLocalStorage.h"
#include "OrbManagement.h"

#include "txClient.h"

log4cxx::LoggerPtr atmiTxInterceptorLogger(log4cxx::Logger::getLogger("TxInterceptor"));

TxInterceptor::TxInterceptor(const char *orbname, IOP::CodecFactory_var cf, const char* name) :
	name_(name), orbname_(strdup(orbname)) {
	// create a GIOP 1.2 CDR encapsulation Codec (to match whatever is in jacorb.properties)
	IOP::Encoding encoding;

	encoding.format = IOP::ENCODING_CDR_ENCAPS;
	encoding.major_version = 1;
	encoding.minor_version = 2;

	this->codec_ = cf->create_codec(encoding);

}
TxInterceptor::~TxInterceptor() {
	if (orbname_ != 0)
		free(orbname_);
}

char* TxInterceptor::name() {
	return CORBA::string_dup(this->name_);
}

char* TxInterceptor::get_control_ior()
{
	return serialize_tx(orbname_);
}

CosTransactions::Control_ptr TxInterceptor::ior_to_control(char * ior)
{
	CORBA::Object_ptr p = atmi_string_to_object(ior, orbname_);

	if (ior != NULL)
		free(ior);

	if (!CORBA::is_nil(p)) {
		CosTransactions::Control_ptr cptr = CosTransactions::Control::_narrow(p);
		CORBA::release(p); // dispose of it now that we have narrowed the object reference
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(), (char *) "narrowed to " << cptr);

		return cptr;
	}

	return NULL;
}

CosTransactions::Control_ptr TxInterceptor::extract_tx_from_context(PortableInterceptor::ServerRequestInfo_ptr ri) {
	if (isTransactional(ri)) {
		return this->ior_to_control(extract_ior_from_context(ri));
	} else {
		return NULL;
	}
}

CosTransactions::Control_ptr TxInterceptor::extract_tx_from_context(PortableInterceptor::ClientRequestInfo_ptr ri) {
	if (isTransactional(ri)) {
		return this->ior_to_control(extract_ior_from_context(ri));
	} else {
		return NULL;
	}
}

char * TxInterceptor::extract_ior_from_context(PortableInterceptor::ServerRequestInfo_ptr ri) {
	try {
		IOP::ServiceContext_var sc = ri->get_request_service_context(tx_context_id);
		CORBA::OctetSeq ocSeq = sc->context_data;
		const char * ior = reinterpret_cast<const char *> (ocSeq.get_buffer());
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(), (char *) "\tserver received ior: " << ior);

		if (ior != NULL)
			return strdup(ior);
	} catch (const CORBA::BAD_PARAM &) {
		// this means that there is no service context for tx_context_id which means that eiter:
		// - the caller is non-transactional, or
		// - the caller is not running in transaction context
	} catch (CORBA::SystemException& ex) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(), (char *) "\tserver service context error");
		throw ;
	}

	return NULL;
}
char * TxInterceptor::extract_ior_from_context(PortableInterceptor::ClientRequestInfo_ptr ri) {
	try {
		IOP::ServiceContext_var sc = ri->get_request_service_context(tx_context_id);
		CORBA::OctetSeq ocSeq = sc->context_data;
		const char * ior = reinterpret_cast<const char *> (ocSeq.get_buffer());
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(), (char *) "\tclient received ior: " << ior);
		if (ior != NULL)
			return strdup(ior);
	} catch (const CORBA::BAD_PARAM &) {
		// this means that there is no service context for tx_context_id which means that eiter:
		// - the caller is non-transactional, or
		// - the caller is not running in transaction context
	} catch (CORBA::SystemException& ex) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(), (char *) "\tclient service context error");
		throw ;
	}

	return NULL;
}

bool TxInterceptor::add_ior_to_context(PortableInterceptor::ClientRequestInfo_ptr ri) {
	if (this->isTransactional(ri)) {
		// add the ior for the transaction to the service contexts
		char *ior = this->get_control_ior();

		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getDebug(), (char*) "\t" << ri->operation() << ": ior is "
				<< (ior==NULL ? "NULL" : "not NULL"));

		if (ior != NULL) {
			IOP::ServiceContext sc;
			int slen = ACE_OS::strlen(ior) + 1;

			sc.context_id = tx_context_id;
			sc.context_data.length(slen);
			CORBA::Octet *sc_buf = sc.context_data.get_buffer();
			ACE_OS::memcpy(sc_buf, ior, slen);
			ri->add_request_service_context(sc, 1);

			CORBA::string_free(ior);
			return true;
		}
	}

	return false;
}

bool TxInterceptor::isTransactional(PortableInterceptor::ClientRequestInfo_ptr ri) {
	try {
		IOP::TaggedComponent_var comp = ri->get_effective_component(AtmiTx::TAG_OTS_POLICY);
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(), (char *) "\tTRANSACTIONAL - " << ri->operation ());
		return true;
    } catch (const CORBA::BAD_PARAM &) {
        ; // not an error means the the intercepted object is not transactional
	} catch (const CORBA::SystemException& ex) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(),
			(char*) "client - isTransactional unexpected CORBA ex: " << ex._name() << (char*) " op=" << ri->operation ());
	} catch (...) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(),
			(char*) "client - isTransactional unexpected ex" << (char*) " op=" << ri->operation ());
	}

	return false;
}

bool TxInterceptor::isTransactional(PortableInterceptor::ServerRequestInfo_ptr ri) {
	try {
		CORBA::Policy_var pv = ri->get_server_policy(AtmiTx::OTS_POLICY_TYPE);
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getTrace(), (char *) "\tTRANSACTIONAL - " << ri->operation ());
		return true;
    } catch (const CORBA::INV_POLICY &) {
		; // not an error - means that the intercepted object is not transactional
	} catch (const CORBA::SystemException& ex) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(),
			(char*) "server - isTransactional unexpected CORBA ex: " << ex._name() << (char*) " op=" << ri->operation ());
	} catch (...) {
		LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getWarn(),
				(char*) "isTransactional unexpected ex, op: " << ri->operation ());
	}

	return false;
}

void TxInterceptor::update_tx_context(PortableInterceptor::ServerRequestInfo_ptr ri) {
	CosTransactions::Control_ptr ctrl = this->extract_tx_from_context(ri);

	if (!CORBA::is_nil(ctrl)) {
		CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) get_control();

		if (!CORBA::is_nil(curr)) {
			if (!curr->_is_equivalent(ctrl)) {
				// Drop the level to info since we are using service queues
				// See comment in CorbaEndpointQueue::send
				LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getInfo(),
						(char*) "\tcurrent and context are different");
			}

			release_control(curr);
		} else {
			associate_tx(ctrl, 0);

			LOG4CXX_LOGLS(atmiTxInterceptorLogger, log4cxx::Level::getDebug(),
				ri->operation () << (char*) ": associated client tx with thread");
		}
	}
}

void TxInterceptor::debug(bool isTx, const char * msg, const char* op) {
	log4cxx::LevelPtr lvl = (isTx ? log4cxx::Level::getDebug() : log4cxx::Level::getTrace());
	LOG4CXX_LOGLS(atmiTxInterceptorLogger, lvl,
			this->name_ << msg << (char*) " " << op << (char*) " transactional="
			<< isTx << " TSS: " << getSpecific(TSS_KEY));
}
void TxInterceptor::debug(PortableInterceptor::ClientRequestInfo_ptr ri, const char* msg) {
//	debug(this->isTransactional(ri), msg, ri->operation());
}

void TxInterceptor::debug(PortableInterceptor::ServerRequestInfo_ptr ri, const char* msg) {
//	debug(this->isTransactional(ri), msg, ri->operation());
}
