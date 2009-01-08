/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
#include <stdlib.h>

#include "xatmi.h"
#include "userlogc.h"
#include "string.h"

void BAR(TPSVCINFO * svcinfo) {
	char* buffer;
	int sendlen;

	userlogc((char*) "bar called  - svc: %s data %s len: %d flags: %d", svcinfo->name, svcinfo->data, svcinfo->len, svcinfo->flags);

	sendlen = XATMI_SERVICE_NAME_LENGTH + 11;
	buffer = tpalloc("X_OCTET", 0, sizeof(char) * sendlen);
	strcpy(buffer, svcinfo->name);
	strcat(buffer, " SAYS HELLO");

	tpreturn(1, 1, buffer, sendlen, 0);
}
