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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xatmi.h"

#include "userlogc.h"

#include "message.h"
#include "AtmiBrokerServerControl.h"

void message_handler(TPSVCINFO * svcinfo) {
	userlogc((char*) "client handler called %s", svcinfo->data);
}

int main(int argc, char **argv) {
    int tpstatus;
	char *retbuf;
	long retbufsize;
    MESSAGE *message;

    if (argc < 4) {
        userlogc("Usage: %s <SERVER_NAME> <REPLY_TO> <MESSAGE>", argv[0]);
        return -1;
    } else {
        userlogc("Running: %s %s %s %s", argv[0], argv[1], argv[2], argv[3]);
    }
	
    // THIS MUST BE DONE TO MAKE SURE THAT THE CLIENT CAN RECEIVE MESSAGES
    setenv("BLACKTIE_SERVER", argv[1], 1);
    setenv("BLACKTIE_SERVER_ID", "1", 1);
    tpstatus = tpadvertise(argv[2], message_handler);
    if (tpstatus == -1 && tperrno != 0) {
        userlogc("Service failed to advertise");
		return -1;
	}

    // CREATE A MESSAGE WITH A REPLY_TO FIELD SET
	message = (MESSAGE*) tpalloc((char*) "X_COMMON", (char*) "message", 0);
    memset(message, tptypes((char*)message, NULL, NULL), '\0');
    memcpy (message->reply_to, argv[2], strlen(argv[2]));
    memcpy (message->data, argv[3], strlen(argv[3]));

    retbufsize = 15;
    retbuf = (char *) tpalloc((char*) "X_OCTET", NULL, retbufsize);

    userlogc("Please press return when you are ready to send the message: %s", message->data);
	getchar();

	tpstatus = tpacall((char*) "FOO", (char*) message, 0, 0);
	
	if (tpstatus == -1 && tperrno == TPENOENT) {
        userlogc("Service failed to operate");
		tpfree((char*)message);
		tpfree(retbuf);
		return -1;
	}

    userlogc("Please press return after you have received a message");
	getchar();
    sleep(2);

	tpfree((char*)message);
	tpfree(retbuf);
	return 0;
}
