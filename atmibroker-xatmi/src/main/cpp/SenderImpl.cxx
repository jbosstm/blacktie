/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
#include "xatmi.h"
#include "SenderImpl.h"

log4cxx::LoggerPtr SenderImpl::logger(log4cxx::Logger::getLogger("SenderImpl"));

SenderImpl::SenderImpl(AtmiBroker::EndpointQueue_var destination) {
	this->destination = destination;
}

SenderImpl::~SenderImpl() {
	LOG4CXX_DEBUG(logger, (char*) "Deleting");
}

void SenderImpl::send(MESSAGE message) {
	int data_size = ::tptypes(message.data, NULL, NULL);
	if (data_size >= 0) {
		if (message.len <= 0 || message.len > data_size) {
			message.len = data_size;
		}

		unsigned char * data_togo = (unsigned char *) malloc(message.len);
		memcpy(data_togo, message.data, message.len);
		AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(message.len, message.len, data_togo, true);
		destination->send(message.replyto, message.rval, message.rcode, aOctetSeq, message.len, message.correlationId, message.flags);
		aOctetSeq = NULL;

		LOG4CXX_DEBUG(logger, (char*) "Called back ");
	} else {
		tperrno = TPEINVAL;
		LOG4CXX_DEBUG(logger, (char*) "A NON-BUFFER WAS ATTEMPTED TO BE SENT");
	}
}

void SenderImpl::close() {
	destination->disconnect();
}