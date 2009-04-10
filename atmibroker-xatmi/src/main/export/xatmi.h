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

#ifndef XATMI_H
#define XATMI_H

#include "atmiBrokerXatmiMacro.h"

#define TPNOBLOCK 0x00000001
#define TPSIGRSTRT 0x00000002
#define TPNOREPLY 0x00000004
#define TPNOTRAN 0x00000008
#define TPTRAN 0x00000010
#define TPNOTIME 0x00000020
#define TPGETANY 0x00000080
#define TPNOCHANGE 0x00000100
#define TPCONV 0x00000400
#define TPSENDONLY 0x00000800
#define TPRECVONLY 0x00001000

#define TPFAIL		0x00000001
#define TPSUCCESS	0x00000002

struct BLACKTIE_XATMI_DLL tpsvcinfo {
#define XATMI_SERVICE_NAME_LENGTH  15
	char name[XATMI_SERVICE_NAME_LENGTH];
	char *data;
	long len;
	long flags;
	int cd;
};
typedef struct BLACKTIE_XATMI_DLL tpsvcinfo TPSVCINFO;

#define X_OCTET		"X_OCTET"
#define X_C_TYPE	"X_C_TYPE"
#define X_COMMON	"X_COMMON"

#define TPEBADDESC 2
#define TPEBLOCK 3
#define TPEINVAL 4
#define TPELIMIT 5
#define TPENOENT 6
#define TPEOS 7
#define TPEPROTO 9
#define TPESVCERR 10
#define TPESVCFAIL 11
#define TPESYSTEM 12
#define TPETIME 13
#define TPETRAN 14
#define TPGOTSIG 15
#define TPEITYPE 17
#define TPEOTYPE 18
#define TPEEVENT 22
#define TPEMATCH 23

#define TPEV_DISCONIMM 0x0001
#define TPEV_SVCERR 0x0002
#define TPEV_SVCFAIL 0x0004
#define TPEV_SVCSUCC 0x0008
#define TPEV_SENDONLY 0x0020

// NOW STARTS THE MAIN OPERATION LIST


#ifdef __cplusplus
extern "C" {
#endif
typedef float(*tpservice)(TPSVCINFO *);

extern BLACKTIE_XATMI_DLL char* tpalloc(char* type, char* subtype, long size); // MEMORY
extern BLACKTIE_XATMI_DLL char* tprealloc(char * addr, long size); // MEMORY
extern BLACKTIE_XATMI_DLL int tpcall(char * svc, char* idata, long ilen, char ** odata, long *olen, long flags); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tpacall(char * svc, char* idata, long ilen, long flags); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tpgetrply(int *idPtr, char ** odata, long *olen, long flags); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tpcancel(int id); // COMMUNICATION
extern BLACKTIE_XATMI_DLL long tptypes(char* ptr, char* type, char* subtype); // MEMORY
extern BLACKTIE_XATMI_DLL void tpfree(char* ptr); // MEMORY
extern BLACKTIE_XATMI_DLL void tpreturn(int rval, long rcode, char* data, long len, long flags); // TJJ ADDED
extern BLACKTIE_XATMI_DLL int tpadvertise(char * svcname, void(*func)(TPSVCINFO *)); // SERVER
extern BLACKTIE_XATMI_DLL int tpunadvertise(char * svcname); // SERVER

extern BLACKTIE_XATMI_DLL int tpsend(int id, char* idata, long ilen, long flags, long *revent); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tprecv(int id, char ** odata, long *olen, long flags, long* event); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tpconnect(char * svc, char* idata, long ilen, long flags); // COMMUNICATION
extern BLACKTIE_XATMI_DLL int tpdiscon(int id); // COMMUNICATION

extern BLACKTIE_XATMI_DLL int* _get_tperrno(void); // CLIENT
extern BLACKTIE_XATMI_DLL long* _get_tpurcode(void); // CLIENT
#ifdef __cplusplus
}
#endif
#define tperrno (*_get_tperrno())										// CLIENT
#define tpurcode (*_get_tpurcode())										// CLIENT
#endif // XATMI_H
