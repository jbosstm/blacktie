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

char prompt(char* prompt) {
	userlogc("Please press return after you: %s...", prompt);
	return getchar();
}

int main(int argc, char **argv) {
	int tpstatus;
	char *retbuf;
	char type[20];
	char subtype[20];
	long retbufsize;
	char *sbuf;
	long sbufsize;
	long callflags;
	int i;
	int response;
	int index;
	int rc = -1;

	rc = tx_open();
	if (rc != 0) {
		userlogc((char*) "open error: %d", rc);
	} else {
		rc = tx_begin();
		if (rc != 0) {
			userlogc((char*) "begin error: %d", rc);
		} else {
			callflags = 0L;
			sbufsize = 29;
			sbuf = tpalloc("X_OCTET", 0, sbufsize);
			memset(sbuf, 0, sbufsize);
			strcpy(sbuf, "Queue me please!");
			retbufsize = 15;
			retbuf = tpalloc("X_OCTET", 0, retbufsize);
			memset(retbuf, 0, retbufsize);

			// tptypes
			tptypes(sbuf, type, subtype);

			// tpcall
			userlogc((char*) "Calling tpcall with input: %s", sbuf);
			tpstatus = tpcall("BAR", sbuf, sbufsize, (char **) &retbuf,
					&retbufsize, callflags);
			userlogc(
					(char*) "Called tpcall with length: %d output: %s and status: %d and tperrno: %d",
					retbufsize, retbuf, tpstatus, tperrno);

			response = prompt("Press 1 to commit or 2 to rollback");
			index = atoi(&response);

			if (index == 1) {
				rc = tx_commit();
				if (rc != 0) {
					userlogc((char*) "commit error: %d", rc);
				}
			} else {
				rc = tx_rollback();
				if (rc != 0) {
					userlogc((char*) "rollback error: %d", rc);
				}
			}

			tpfree(sbuf);
			tpfree(retbuf);
		}
	}
	rc = tx_close();
	if (rc != 0) {
		userlogc((char*) "close error: %d", rc);
	}
	return rc;
}
