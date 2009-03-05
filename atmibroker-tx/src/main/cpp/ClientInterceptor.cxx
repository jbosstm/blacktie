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
#include "ClientInterceptor.h"

#include "tao/OctetSeqC.h"
#include "tao/LocalObject.h"
#include "tao/PI/ClientRequestInfo.h"

#include <iostream>

log4cxx::LoggerPtr atmiClientInterceptorlogger(log4cxx::Logger::getLogger("ClientInterceptor"));

ClientInterceptor::ClientInterceptor(const char *orbname, IOP::CodecFactory_var cf) :
	TxInterceptor(orbname, cf, "ATMIClientTxInterceptor") {}

void ClientInterceptor::send_poll(PortableInterceptor::ClientRequestInfo_ptr ri) {
}

void ClientInterceptor::send_request(PortableInterceptor::ClientRequestInfo_ptr ri) {
	debug(ri, ":send_request");
	this->add_ior_to_context(ri);
}

void ClientInterceptor::receive_reply(PortableInterceptor::ClientRequestInfo_ptr ri) {
	debug(ri, ":receive_reply");
}

void ClientInterceptor::receive_other(PortableInterceptor::ClientRequestInfo_ptr ri) {
	debug(ri, ":receive_other");
}

void ClientInterceptor::receive_exception(PortableInterceptor::ClientRequestInfo_ptr ri) {
	debug(ri, ":receive_exception");
}

void ClientInterceptor::resume_tx(PortableInterceptor::ClientRequestInfo_ptr ri) { }

long ClientInterceptor::find_control(CORBA::ULong requestId, bool erase) { return 0L; }
