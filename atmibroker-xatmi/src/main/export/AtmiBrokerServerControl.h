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


#ifndef BLACKTIE_SERVERCONTROL_H_
#define BLACKTIE_SERVERCONTROL_H_

#include "atmiBrokerXatmiMacro.h"
#include "AtmiBrokerEnvXml.h"
#include "xatmi.h"

typedef void (*SVCFUNC)(TPSVCINFO *);

struct BLACKTIE_XATMI_DLL _service_status {
	ServiceInfo* service;
	bool    status;
	SVCFUNC func;
};

typedef struct BLACKTIE_XATMI_DLL _service_status ServiceStatus;

#ifdef __cplusplus
extern "C" {
#endif
extern BLACKTIE_XATMI_DLL int serverinit(int argc, char** argv);
extern BLACKTIE_XATMI_DLL int serverrun();
extern BLACKTIE_XATMI_DLL int serverdone();
extern BLACKTIE_XATMI_DLL void server_sigint_handler_callback(int sig_type);
extern BLACKTIE_XATMI_DLL int isadvertised(char* name);
extern BLACKTIE_XATMI_DLL int advertiseByAdmin(char* name);
#ifdef __cplusplus
}
#endif

#endif
