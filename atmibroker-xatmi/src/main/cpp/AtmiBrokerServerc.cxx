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
#ifdef TAO_COMP
#include <tao/ORB.h>
#endif

#include "AtmiBrokerServer.h"
#include "AtmiBrokerServerControl.h"

extern "C" {
#include "xatmi.h"
}

#include "userlog.h"
//TODO READD #include "extensions.h"

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerServerc(Logger::getLogger("AtmiBrokerServerc"));

int tpadvertise(char * svcname, void(*func)(TPSVCINFO *)) {
	tperrno = 0;
	int toReturn = -1;
	if (ptrServer) {
		if (ptrServer->advertiseService(svcname, func)) {
			toReturn = 0;
		}
	} else {
		tperrno = TPEPROTO;
	}
	return toReturn;
}

int tpunadvertise(char * svcname) {
	tperrno = 0;
	int toReturn = -1;
	if (ptrServer) {
		if (svcname && strcmp(svcname, "") != 0) {
			if (ptrServer->isAdvertised(svcname)) {
				ptrServer->unadvertiseService(svcname);
				toReturn = 0;
			} else {
				tperrno = TPENOENT;
			}
		} else {
			tperrno = TPEINVAL;
		}
	} else {
		tperrno = TPEPROTO;
	}
	return toReturn;
}
