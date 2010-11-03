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

#include "xatmi.h"
#include "btxatmi.h"

#include "userlogc.h"

#ifdef WIN32
#include "windows.h"
#endif

static char* SERVICE = (char*) "TestOne";
static int msgCnt = 0;

/*
 * keep servicing messages until the target msgCnt is reached after which disable message
 * delivery by unadvertising SERVICE
 */
static void qservice(TPSVCINFO *svcinfo) {
	userlogc((char*) "svc: %s data: %s len: %d flags: %d", svcinfo->name, svcinfo->data, svcinfo->len, svcinfo->flags);
	msgCnt -= 1;

	if (msgCnt <= 0) {
		int err = tpunadvertise(SERVICE);

		if (tperrno != 0 || err == -1)
			userlogc((char*) "unadvertise error: %d %d", err, tperrno);
	}
}

/**
 * Send a single message with the given id at the requested priority.
 * If pri is -1 then do not use priorities.
 */
static int send_one(int id, int pri) {
    msg_opts_t mopts;
    char msg[16];
    char* buf;
    long len;
    int err = -1;

    mopts.priority = pri;
    (void) sprintf(msg, (char*) "%d", id);
    len = strlen(msg) + 1;

    if (pri < 0)
        buf = tpalloc((char*) "X_OCTET", NULL, len);
    else
        buf = btalloc(&mopts, (char*) "X_OCTET", NULL, len);

    if (tperrno != 0) {
		userlogc((char*) "tp alloc error: %d", tperrno);
	} else {
    	(void) strcpy(buf, msg);
    	err = tpacall(SERVICE, buf, len, TPNOREPLY);

    	if (tperrno != 0 || err != 0)
			userlogc((char*) "tpacall error: %d %d", err, tperrno);

    	tpfree(buf);
	}

	return err;
}

/**
 * Send cnt messages begining with message id msgid with the requested priority.
 */
static int put_messages(unsigned int cnt, unsigned int msgid, unsigned int pri) {
	int i;
	int err;

	for (i = msgid; i < msgid + cnt; i++)
		if ((err = send_one(i, pri)) != 0)
			return err;

	return 0;
}

/**
 * Take the next cnt messages off the service queue.
 */
static int get_messages(unsigned int cnt) {
	int err;
	int maxSleep = 10;

	/*
	 * Register a service listener for the queue. If the env variable BLACKTIE_SERVER_ID
	 * is set then the framework will automatically register a server listener for the
	 * queue during tpadvertise. Otherwise do it manually using serverinit(argc, argv)
	 * where argv includes, for example, server -c linux -i 1
	 */
	msgCnt = cnt;
	err = tpadvertise(SERVICE, qservice);

	if (tperrno != 0 || err == -1) {
		userlogc((char*) "advertise error: %d %d", err, tperrno);
		return -1;
	}

	// wait for the service routine, qservice, to consume the required number of messages (msgCnt)
	while (msgCnt > 0 && maxSleep-- > 0)
#ifndef WIN32
		if (sleep(1) != 0)
			break;
#else
		Sleep(1000);
#endif

	// Manually shutdown the server. TODO have the framework shut it down on the final tpunadvertise
	serverdone();

	if (maxSleep == 0 || msgCnt > 0)
		return -1;

	return 0;
}

/*
 * An example showing how to decouple sender and receiver using queues with optional priorities.
 */
int main(int argc, char **argv) {
	int rc = -1;

	if (argc < 3) {
		userlogc((char *) "usage: %s <put cnt [msgid priority] | get cnt>", argv[0]);
	} else if (strcmp(argv[1], "put") == 0) {
		rc = put_messages(atoi(argv[2]), argc > 3 ? atoi(argv[3]) : 0, argc > 4 ? atoi(argv[4]) : -1);
	} else if (strcmp(argv[1], "get") == 0) {
		rc = get_messages(atoi(argv[2]));
	} else {
		userlogc((char *) "usage: %s <put cnt [priority] | get cnt>", argv[0]);
	}

	return rc;
}
