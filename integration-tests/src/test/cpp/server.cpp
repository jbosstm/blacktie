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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include "string.h"

#include "AtmiBrokerServerControl.h"
#include "xatmi.h"
#include "userlogc.h"

extern "C"void BAR(TPSVCINFO * svcinfo) {
	int sendlen = 14;
	char* buffer = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strncpy(buffer, "BAR SAYS HELLO", 14);

	tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
}

extern "C"void TestTPACall(TPSVCINFO * svcinfo) {
	if (strncmp(svcinfo->data, "echo", svcinfo->len) == 0) {
		userlogc((char*) "Returning goodness");
		int sendlen = 16;
		char* buffer = tpalloc((char*) "X_OCTET", NULL, sendlen);
		strncpy(buffer, "helloTestTPACall", 16);
		tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
	} else {
		userlogc((char*) "Returning badness");
		tpreturn(TPFAIL, 1, svcinfo->data, svcinfo->len, 0);
	}
}

extern "C"void tpcall_x_octet(TPSVCINFO * svcinfo) {
	int sendlen = 14;
	char* buffer = tpalloc((char*) "X_OCTET", 0, sendlen);
	strncpy(buffer, "BAR SAYS HELLO", 14);
	tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
}

struct test_t {
	int foo;
	short bar;
	long foobar;
	char z;
	float floater;
	double doubley;
	char status[128];
};
typedef struct test_t TEST;

extern "C"void tpcall_x_c_type(TPSVCINFO * svcinfo) {
	TEST* buffer = (TEST*) tpalloc((char*) "X_C_TYPE", (char*) "deposit", NULL);
	buffer->foo = 222;
	buffer->bar = 33;
	buffer->foobar = 11;
	buffer->z = 'c';
	buffer->floater = 444.97;
	buffer->doubley = 7.7;
	strcpy(buffer->status, "tpcall_x_c_type");
	tpreturn(TPSUCCESS, 1, (char*) buffer, 0, 0);
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_serverinit(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverinit called");
#ifdef WIN32
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"win32", (char*)"foo"};
#else
	char* argv[] = {(char*)"server", (char*)"-c", (char*)"linux", (char*)"foo"};
#endif
	int argc = sizeof(argv)/sizeof(char*);

	exit_status = serverinit(argc, argv);
	exit_status = tpadvertise((char*) "BAR", BAR);
	exit_status = tpadvertise((char*) "tpcall_x_octet", tpcall_x_octet);
	exit_status = tpadvertise((char*) "tpcall_x_c_type", tpcall_x_c_type);
	exit_status = tpadvertise((char*) "TestTPACall", TestTPACall);
	userlogc((char*) "serverinit returning");
	return;
}

extern "C"
JNIEXPORT void JNICALL Java_org_jboss_blacktie_jatmibroker_RunServer_serverdone(JNIEnv *, jobject) {
	int exit_status = -1;
	userlogc((char*) "serverdone called");
	exit_status = serverdone();
	userlogc((char*) "serverdone returning");
	return;
}
