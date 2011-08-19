/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
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

#include "CodecFactory.h"
#include "DefaultCodecImpl.h"

#include <exception>
#include "malloc.h"

#include "AtmiBrokerEnvXml.h"

log4cxx::LoggerPtr CodecFactory::logger(log4cxx::Logger::getLogger(
		"CodecFactory"));

CodecFactory::CodecFactory() {
	LOG4CXX_DEBUG(logger, (char*) "CodecFactory");
	default_impl = new DefaultCodecImpl();
}

CodecFactory::~CodecFactory() {
	LOG4CXX_DEBUG(logger, (char*) "deconstruct CodecFactory");
	if(default_impl != NULL) {
		LOG4CXX_DEBUG(logger, (char*) "delete default_impl");
		delete default_impl;
	}
}

Codec* CodecFactory::getCodec(char* name) {
	return default_impl;
}
