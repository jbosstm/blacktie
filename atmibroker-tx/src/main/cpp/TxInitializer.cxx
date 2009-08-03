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
#include "TxInitializer.h"
#include "ServerInterceptor.h"
#include "ClientInterceptor.h"
#include "TxIORInterceptor.h"

#include "OTSPolicyFactory.h"
#include "TxPolicyC.h"

// includes for registering interceptor with the orb
#include "tao/ORBInitializer_Registry.h"
#include "tao/PI_Server/PI_Server.h"

#include "tao/ORB_Constants.h"
#include "ace/OS_NS_string.h"

#include "log4cxx/logger.h"

TxInitializer* TxInitializer::instance = NULL;



log4cxx::LoggerPtr loggerTxInitializer(log4cxx::Logger::getLogger("TxInitializer"));
TxInitializer* TxInitializer::get_instance() {
	if (instance == NULL) {
		instance = new TxInitializer();
		PortableInterceptor::register_orb_initializer(instance);
	}
	return instance;
}

TxInitializer::TxInitializer() : orbname_(0) {
}

void TxInitializer::set_orb(const char * name) {
	if (orbname_ != 0)
		free(orbname_);
	orbname_ = strdup(name);
}

void TxInitializer::pre_init(PortableInterceptor::ORBInitInfo_ptr info) {
}

void TxInitializer::post_init(PortableInterceptor::ORBInitInfo_ptr info) {
	LOG4CXX_LOGLS(loggerTxInitializer, log4cxx::Level::getDebug(), (char*) "");

	// register policy factories
	PortableInterceptor::PolicyFactory_ptr p;

	ACE_NEW_THROW_EX(p, OTSPolicyFactory,
			CORBA::NO_MEMORY(CORBA::SystemException::_tao_minor_code(TAO::VMCID, ENOMEM), CORBA::COMPLETED_NO));

	PortableInterceptor::PolicyFactory_var pf(p);
	info->register_policy_factory(AtmiTx::OTS_POLICY_TYPE, pf.in());

	IOP::CodecFactory_var cf = info->codec_factory();

	// register IOR interceptors
	PortableInterceptor::IORInterceptor_ptr iori;
	iori = new (ACE_nothrow) TxIORInterceptor(orbname_, cf);
	if (iori == 0)
		throw CORBA::NO_MEMORY(CORBA::SystemException::_tao_minor_code(TAO::VMCID, ENOMEM), CORBA::COMPLETED_NO);

//	PortableInterceptor::IORInterceptor_var ior_interceptor(iori);
	PortableInterceptor::IORInterceptor_var ior_interceptor = iori;
	info->add_ior_interceptor(ior_interceptor.in());

	// register server side interceptors.
	PortableInterceptor::ServerRequestInterceptor_var si = new ServerInterceptor(orbname_, cf);
	info->add_server_request_interceptor(si.in());

	// register client side interceptors.
	PortableInterceptor::ClientRequestInterceptor_ptr ci = new ClientInterceptor(orbname_, cf);
	PortableInterceptor::ClientRequestInterceptor_var civ = ci;
	info->add_client_request_interceptor(civ.in());

	LOG4CXX_LOGLS(loggerTxInitializer, log4cxx::Level::getDebug(), (char*) "out");
}

void register_tx_interceptors(const char *name) {
	TxInitializer::get_instance()->set_orb(name);
}
