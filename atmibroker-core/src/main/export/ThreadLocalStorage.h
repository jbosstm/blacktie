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
#ifndef ThreadLocalStorage_H
#define ThreadLocalStorage_H

#include "atmiBrokerCoreMacro.h"

// key for storing control in thread specific storage
const int TSS_KEY = 0xaf; // key for accessing OTS control
const int SVC_KEY = 0xb1; // key for accessing atmi service
const int SVC_SES = 0xb4; // key for accessing service session
const int TPE_KEY = 0xa5; // key for accessing tperrno
// define other keys here

extern BLACKTIE_CORE_DLL int getKey();
extern BLACKTIE_CORE_DLL bool setSpecific(int key, void* threadData);
extern BLACKTIE_CORE_DLL void* getSpecific(int key);
extern BLACKTIE_CORE_DLL bool destroySpecific(int key);

extern char* TSS_TPERESET;
extern char* TSS_TPEBADDESC;
extern char* TSS_TPEBLOCK;
extern char* TSS_TPEINVAL;
extern char* TSS_TPELIMIT;
extern char* TSS_TPENOENT;
extern char* TSS_TPEOS;
extern char* TSS_TPEPROTO;
extern char* TSS_TPESVCERR;
extern char* TSS_TPESVCFAIL;
extern char* TSS_TPESYSTEM;
extern char* TSS_TPETIME;
extern char* TSS_TPETRAN;
extern char* TSS_TPGOTSIG;
extern char* TSS_TPEITYPE;
extern char* TSS_TPEOTYPE;
extern char* TSS_TPEEVENT;
extern char* TSS_TPEMATCH;

#endif //ThreadLocalStorage_H
