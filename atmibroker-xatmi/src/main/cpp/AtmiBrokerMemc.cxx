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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

extern "C" {
#include "xatmi.h"
}

#include "AtmiBrokerMem.h"
#include "userlog.h"
#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerMemc(Logger::getLogger("AtmiBrokerMemc"));

char* tpalloc(char* type, char* subtype, long size) {
	userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "tpalloc - type: '%s' size: %d", type, size);
	return AtmiBrokerMem::get_instance()->tpalloc(type, subtype, size);
}

char* tprealloc(char * addr, long size) {
	tperrno = 0;
	userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "tpralloc - addr: %p size: %d", addr, size);

	return AtmiBrokerMem::get_instance()->tprealloc(addr, size);
}

void tpfree(char* ptr) {
	tperrno = 0;
	if (ptr) {
		userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "tpfree - ptr: %p", ptr);
		AtmiBrokerMem::get_instance()->tpfree(ptr);
	} else {
		userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "tpfree - null ptr");
	}
}

long tptypes(char* ptr, char* type, char* subtype) {
	tperrno = 0;
	userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "tptypes - ptr: %p %s", ptr, type);

	return AtmiBrokerMem::get_instance()->tptypes(ptr, type, subtype);
}

void freeAllMemory() {
	userlog(Level::getDebug(), loggerAtmiBrokerMemc, (char*) "freeAllMemory");

	AtmiBrokerMem::get_instance()->freeAllMemory();
}

