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

#include "xatmi.h"
#include "tx.h"

#include "userlogc.h"

int main(int argc, char **argv) {
	int txstatus;
	int tpstatus;
	char *rbuf;
	char *retbuf;
	char type[20];
	char subtype[20];
	long rbufsize;
	long retbufsize;
	char *sbuf;
	long sbufsize;
	long callflags;

	txstatus = tx_open();
	if (txstatus != TX_OK) {
		userlogc((char*) "ERROR - Could not open transaction: ");
	}
	txstatus = tx_begin();
	if (txstatus != TX_OK) {
		userlogc((char*) "ERROR - Could not begin transaction: ");
	}
	callflags = 0;
	sbufsize = 71 + XATMI_SERVICE_NAME_LENGTH + 1;
	sbuf = tpalloc("X_OCTET", 0, sbufsize);
	strcpy(sbuf, "SERVICE - THIS IS YOUR CLIENT SPEAKING");
	rbufsize = 0;
	retbufsize = 0;

	if ((rbuf = (char *) tpalloc((char*) "X_OCTET", NULL, sbufsize)) != NULL) {

		// tptypes
		tptypes(rbuf, type, subtype);

		// tpcall
		strcpy(rbuf, "BAR");
		strcat(rbuf, sbuf);
		rbufsize = strlen(rbuf);

		userlogc((char*) "Calling tpcall with input: %s", rbuf);
		tpstatus = tpcall("BAR", rbuf, rbufsize, (char **) &retbuf,
				&retbufsize, callflags);
		userlogc((char*) "Called tpcall with output: %s and status: %d",
				retbuf, tpstatus);

		tpfree(rbuf);
	} else {
		userlogc((char*) "ERROR - Could not allocate a buffer");
	}
	txstatus = tx_commit();
	if (txstatus != TX_OK) {
		userlogc((char*) "ERROR - Could not commit transaction: ", rbuf);
	}
	txstatus = tx_close();
	if (txstatus != TX_OK) {
		userlogc((char*) "ERROR - Could not close transaction: ", rbuf);
	}

	return 0;
}
