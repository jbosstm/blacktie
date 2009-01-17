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

#include "xatmi.h"
#include "AtmiBroker.h"
#include "AtmiBrokerConversation.h"
#include "AtmiBroker_ServiceImpl.h"
#include "userlog.h"

extern "C" {
#include "AtmiBrokerClientControl.h"
}

#include "log4cxx/logger.h"
using namespace log4cxx;
using namespace log4cxx::helpers;
LoggerPtr loggerAtmiBrokerConversationc(Logger::getLogger("AtmiBrokerConversationc"));

extern void* getSpecific(int key);

int tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen, long flags) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpcall(svc, idata, ilen, odata, olen, flags);
	else
		return -1;
}

int tpacall(char * svc, char* idata, long ilen, long flags) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpacall(svc, idata, ilen, flags);
	else
		return -1;
}

int tpconnect(char * svc, char* idata, long ilen, long flags) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpconnect(svc, idata, ilen, flags);
	else
		return -1;
}

int tpsend(int id, char* idata, long ilen, long flags, long *revent) {
	AtmiBroker_ServiceImpl* thread = (AtmiBroker_ServiceImpl*) getSpecific(1);
	if (thread == NULL)
		if (clientinit() != -1)
			return AtmiBrokerConversation::get_instance()->tpsend(id, idata, ilen, flags, revent);
		else
			return -1;
	else
		thread->tpsend(id, idata, ilen, flags, revent);
}

int tprecv(int id, char ** odata, long *olen, long flags, long* event) {
	AtmiBroker_ServiceImpl* thread = (AtmiBroker_ServiceImpl*) getSpecific(1);
	if (thread == NULL)
		if (clientinit() != -1)
			return AtmiBrokerConversation::get_instance()->tprecv(id, odata, olen, flags, event);
		else
			return -1;
	else
		thread->tprecv(id, odata, olen, flags, revent);

}

int tpdiscon(int id) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpdiscon(id);
	else
		return -1;
}

int tpcancel(int id) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpcancel(id);
	else
		return -1;
}

int tpgetrply(int *id, char ** odata, long *olen, long flags) {
	if (clientinit() != -1)
		return AtmiBrokerConversation::get_instance()->tpgetrply(id, odata, olen, flags);
	else
		return -1;
}

void tpreturn(int rval, long rcode, char* data, long len, long flags) {
	// TJJ
	AtmiBroker_ServiceImpl* thread = (AtmiBroker_ServiceImpl*) getSpecific(1);
	thread->tpreturn(rval, rcode, data, len, flags);
}
