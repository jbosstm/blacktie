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
#include <jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include "string.h"

#include "AtmiBrokerServerControl.h"
#include "xatmi.h"

extern "C"void BAR(TPSVCINFO * svcinfo) {
	int sendlen = 60;
	char* buffer = tpalloc("X_OCTET", 0, sendlen);
	strcpy(buffer, svcinfo->name);
	strcat(buffer, " SAYS HELLO");

	tpreturn(1, 1, buffer, sendlen, 0);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_core_RunServer_serverinit(JNIEnv *, jobject) {
	int exit_status = -1;
	exit_status = serverinit();
	exit_status = tpadvertise("BAR", BAR);
	return;
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_core_RunServer_serverdone(JNIEnv *, jobject) {
	int exit_status = -1;
	exit_status = serverdone();
	return;
}
