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
JNIEXPORT jint JNICALL Java_org_jboss_blacktie_btadmin_server_RunServer_serverinit(JNIEnv * env, jobject, jstring serverNameIn, jstring idIn) {
	int exit_status = -1;
	jboolean isCopy;
	const char* serverName;
	const char* id;

	serverName = (env)->GetStringUTFChars(serverNameIn, &isCopy);
	id = (env)->GetStringUTFChars(idIn, &isCopy);
	userlogc((char*) "serverinit called: %s %s", serverName, id);

#ifdef WIN32
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"win32", (char*)serverName, (char*)"-i", (char*)id};
#else
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"linux", (char*)serverName, (char*)"-i", (char*)id};
#endif
	int argc = sizeof(argv)/sizeof(char*);

	exit_status = serverinit(argc, argv);
	exit_status = tpadvertise((char*) "BAR", BAR);

	if (exit_status != 0) {
		// Try to shut down the server
		serverdone();
	}
	(env)->ReleaseStringUTFChars(serverNameIn , serverName); // release jstring
	(env)->ReleaseStringUTFChars(idIn , id); // release jstring
	userlogc((char*) "serverinit returning");
	return exit_status;
}

extern "C"
JNIEXPORT jint JNICALL Java_org_jboss_blacktie_btadmin_server_RunServer_serverdone(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverdone called");
	exit_status = serverdone();
	userlogc((char*) "serverdone returning");
	return exit_status;
}
