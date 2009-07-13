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
#include <stdarg.h>

#include "xatmi.h"

#include "userlogc.h"
#include "request.h"

/* log a message */
void logit(int debug, const char * format, ...) {
	char str[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(str, sizeof (str), format, args);
	va_end(args);

	if (debug)
		userlogc_debug(str);
	else
		userlogc(str);
}

char * get_buf(const char *data, const char *dbfile, enum TX_TYPE txtype) {
	char type[20];
	char subtype[20];
	test_req_t req;
	char *sbuf;

	strcpy(req.data, data);
	strcpy(req.db, dbfile);
	req.txtype = txtype;

	sbuf = tpalloc("X_OCTET", 0, sizeof (test_req_t));
	memcpy(sbuf, &req, sizeof (test_req_t));

	// tptypes
	tptypes(sbuf, type, subtype);

	return sbuf;
}

void init_req(test_req_t *req, int prodid, const char *dbfile, const char *data, char op, enum TX_TYPE txtype) {
	req->prod = prodid;
	req->txtype = txtype;
	req->op = op;
	req->status = 0;
	(void) strcpy(req->data, data);
	(void) strcpy(req->db, dbfile);
}

test_req_t * get_tbuf(const char *data, const char *dbfile, char op, enum DB_TYPE prod, enum TX_TYPE txtype) {
	test_req_t *req = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));

	strcpy(req->data, data);
	strcpy(req->db, dbfile);
	req->txtype = txtype;
	req->prod = prod;
	req->op = op;
	req->status = 0;

	return req;
}

int fail(const char *reason, int ret)
{
	fprintf(stderr, "%s: %d\n", reason, ret);
	return ret;
}

void fatal(const char *msg)
{
	logit(0, msg);
	exit (-1);
}

int null_access(test_req_t *req, test_req_t *resp)
{
	resp->status = 0;

	return 0;
}
