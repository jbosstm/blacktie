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
#include <jni.h>

#include "AtmiBrokerServerControl.h"
#include "xatmi.h"
#include "userlogc.h"

extern "C" void BAR(TPSVCINFO *svcinfo);

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_btadmin_server_RunServer_serverinit(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverinit called");
#ifdef WIN32
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"win32", (char*)"default", (char*)"-i", (char*)"1"};
#else
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"linux", (char*)"default", (char*)"-i", (char*)"1"};
#endif
	int argc = sizeof(argv)/sizeof(char*);

	exit_status = serverinit(argc, argv);
	exit_status = tpadvertise((char*) "BAR", BAR);
	userlogc((char*) "serverinit returning");
	return;
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_btadmin_server_RunServer_serverdone(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverdone called");
	exit_status = serverdone();
	userlogc((char*) "serverdone returning");
	return;
}
