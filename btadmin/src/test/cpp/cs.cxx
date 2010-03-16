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
#include "AtmiBrokerServerControl.h"

#include "userlogc.h"

#include "xatmi.h"

void BAR(TPSVCINFO * svcinfo) {
	int sendlen;
	char* buffer;
	sendlen = 14;
	buffer = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strncpy(buffer, "BAR SAYS HELLO", 14);
	tpreturn(TPSUCCESS, 1, buffer, sendlen, 0);
}
//SERVICE_DEFINITIONS

int main(int argc, char **argv) {
	int exit_status = serverinit(argc, argv);

	if (exit_status != -1) {
		tpadvertise("BAR", BAR);
        //SERVICE_ADVERTISEMENTS
		exit_status = serverrun();
	} else {
		userlogc((char*) "error initialising server");
	}
	serverdone();
	return exit_status;
}
