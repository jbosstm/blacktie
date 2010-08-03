/*
 * JBoss, Home of Professional Open Source
 * Copyright 2010, Red Hat, Inc., and others contributors as indicated
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "xatmi.h"

#include "userlogc.h"

static char* SERVICE = (char*) "TestOne";
static int msgCnt = 0;

static void qservice(TPSVCINFO *svcinfo) {
	userlogc((char*) "svc: %s data: %s len: %d flags: %d", svcinfo->name, svcinfo->data, svcinfo->len, svcinfo->flags);
	msgCnt -= 1;

	if (msgCnt <= 0) {
		int err = tpunadvertise(SERVICE);

		if (tperrno != 0 || err == -1)
			userlogc((char*) "unadvertise error: %d %d", err, tperrno);
	}
}

static void put_messages(unsigned int cnt) {
	while (cnt-- != 0) {
		char msg[80];
		sprintf(msg, (char*) "request %d", cnt);
		long sendlen = strlen(msg) + 1;
		char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
		(void) strcpy(sendbuf, msg);
		int cd = tpacall(SERVICE, sendbuf, sendlen, TPNOREPLY);

		userlogc((char*) "tpacall returned %d %d", cd, tperrno);

		tpfree(sendbuf);
	}
}

static void get_messages(unsigned int cnt) {
	int err = 0;
	// Register a service listener for the queue. If the env variable BLACKTIE_SERVER_ID
	// is set then the framework will automatically register a server listener for the
	// queue during tpadvertise. Otherwise do it manually as follows:
	// char* argv[] = {(char*)"server", (char*)"-c", (char*)"linux", (char*)"-i", (char*)"1"};
	// serverinit(sizeof(argv)/sizeof(char*), argv);

	msgCnt = cnt;
	err = tpadvertise(SERVICE, qservice);

	if (tperrno != 0 || err == -1)
		userlogc((char*) "advertise error: %d %d", err, tperrno);

	// wait for the service routine, qservice, to clear msgCnt
	while (msgCnt > 0)
		(void) sleep(1);

	// Manually shutdown the server. TODO have the framework shut it down on the final tpunadvertise
	serverdone();
}

static void fatal(int argc, char **argv) {
	userlogc((char *) "usage: %s <put msgCnt | get msgCnt>", argv[0]);
	exit(0);
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fatal(argc, argv);
	} else if (strcmp(argv[1], "put") == 0) {
		put_messages(atoi(argv[2]));
	} else if (strcmp(argv[1], "get") == 0) {
		get_messages(atoi(argv[2]));
	} else {
		fatal(argc, argv);
	}

	return 0;
}
