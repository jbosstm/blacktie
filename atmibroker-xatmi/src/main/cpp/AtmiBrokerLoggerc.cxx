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

#ifdef TAO_COMP
#include <tao/ORB.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "userlog.h"
extern "C" {
#include "userlogc.h"
}

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerLogc(Logger::getLogger("AtmiBrokerLogc"));

extern "C"ATMIBROKER_DLL
int userlogc(char * format, va_list *pargs) {
	return userlog(Level::getInfo(), loggerAtmiBrokerLogc, format, pargs);
}

int userlog(const LevelPtr& level, LoggerPtr& logger, char * format, ...) {
	if (logger->isEnabledFor(level)) {
		char str[2048];
		va_list args;
		va_start(args, format);
		vsprintf(str, format, args);
		va_end(args);
		LOG4CXX_LOGLS(logger, level, str);
	}
	return 1;
}
