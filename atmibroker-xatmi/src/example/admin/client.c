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

void output(char* operationName, char* list) {
	userlogc("Output from %s: ", operationName);

	char* nextToken = strtok(list, ",");
	int i = 0;
	while (nextToken != NULL) {
		userlogc((char*) "Element: %d Value: %s", i, nextToken);
		nextToken = strtok(NULL, " ");
		i++;
	}
}

int main(int argc, char **argv) {
	int tpstatus;
	char *retbuf;
	long retbufsize;
	char *sbuf;
	long sbufsize;
	long callflags;
	callflags = 0;
	sbufsize = 20;
	sbuf = tpalloc("X_OCTET", 0, sbufsize);
	memset(sbuf, 0, sbufsize);
	strcpy(sbuf, "listRunningServers,");
	retbufsize = 1;
	retbuf = tpalloc("X_OCTET", 0, retbufsize);

	prompt("Start JBoss Application Server");
	prompt("Start an XATMI server");

	// listRunningServers
	tpstatus = tpcall("BTDomainAdmin", sbuf, sbufsize, (char **) &retbuf,
			&retbufsize, callflags);
	char* noTrailingBar = (char*) malloc(retbufsize);
	strncpy(noTrailingBar, retbuf, retbufsize - 1);
	noTrailingBar[retbufsize - 1] = NULL;
	output((char*) "listRunningServers", noTrailingBar);

	//	userlogc(
	//			(char*) "Called tpcall with length: %d output: %s and status: %d and tperrno: %d",
	//			retbufsize, retbuf, tpstatus, tperrno);


	tpfree(sbuf);
	tpfree(retbuf);
	return 0;
}
