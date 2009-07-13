/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#include <stdlib.h>
#include <string.h>

#include "xatmi.h"
#include "userlogc.h"

#include "client.h"

void DBS(TPSVCINFO * svcinfo) {
	test_req_t *req = (test_req_t *) svcinfo->data;
	test_req_t *resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	product_t *p = products;

	userlogc((char*) "Service DBS: prod=%d op=%c data=%s dbf=%s", req->prod, req->op, req->data, req->db);

	resp->status = -1;
	snprintf(resp->data, sizeof (resp->data), "Unsupported database: %s", req->db);
	for (p = products; p->id != -1; p++) {
		if (req->prod == p->id) {
			resp->status = p->access(req, resp);
			break;
		}
	}

	tpreturn(1, 1, (char *) resp, sizeof (test_req_t), 0);
}
