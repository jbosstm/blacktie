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

#include <xatmi.h>
#include <tx.h>
#include <userlogc.h>

#include "tx/request.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

/* helper methods for controling transactions */
int is_begin(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_BEGIN || txtype == TX_TYPE_BEGIN_COMMIT || txtype == TX_TYPE_BEGIN_ABORT);
}
int is_commit(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_COMMIT || txtype == TX_TYPE_BEGIN_COMMIT);
}
int is_abort(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_ABORT || txtype == TX_TYPE_BEGIN_ABORT);
}
int start_tx(enum TX_TYPE txtype) {
	if (is_begin(txtype)) {
		logit(1, (char*) "- Starting Transaction");
		if (tx_begin() != TX_OK) {
			logit(0, (char*) "TX ERROR - Could not begin transaction: ");
			return -1;
		}
	}

	return 0;
}
int end_tx(enum TX_TYPE txtype) {
	int rv = 0;
	if (is_commit(txtype)) {
		logit(1, (char*) "- Commiting transaction");
		if (tx_commit() != TX_OK)
			rv = -1;
	} else if (is_abort(txtype)) {
		logit(1, (char*) "- Rolling back transaction");
		if (tx_rollback() != TX_OK)
			rv = -1;
	}

	if (rv != 0)
		logit(0, (char*) "TX ERROR - Could not terminate transaction");

	return rv;
}

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

static int reqid = 0;
static void _init_req(test_req_t *req, int prodid, const char *dbfile, const char *data, char op, enum TX_TYPE txtype, int expect) {
	req->prod = prodid;
	req->txtype = txtype;
	req->expect = expect;
	req->id = ++reqid;
	req->op = op;
	req->status = 0;
	req->data[0] = 0;
	req->db[0] = 0;

	if (data)
		(void) strncpy(req->data, data, sizeof(req->data));
	if (dbfile)
		(void) strncpy(req->db, dbfile, sizeof(req->db));
}

test_req_t * get_buf(int remote, const char *data, const char *dbfile, char op, int prod, enum TX_TYPE txtype, int expect) {
	test_req_t *req;

	if (remote)
		req = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	else
		req = (test_req_t *) malloc(sizeof (test_req_t));

	if (req == NULL) {
		/*fatal("out of memory (for alloc)");*/
		logit(0, "out of memory (for alloc)");

		return NULL;
	}

	_init_req(req, prod, dbfile, data, op, txtype, expect);

	return req;
}

void free_buf(int remote, test_req_t *req) {
	if (remote)
		tpfree((char *) req);
	else
		free(req);
}

int fail(const char *reason, int ret)
{
	logit(0, "%s: %d\n", reason, ret);
	return ret;
}

int fatal(const char *msg)
{
	logit(0, msg);
	/*exit (-1);*/
	return -1;
}

long null_xaflags()
{
	return 0L;
}

int null_access(test_req_t *req, test_req_t *resp)
{
	resp->status = 0;
	snprintf(resp->data, sizeof(resp->data), "%d", req->expect);

	logit(1, "null_access: prod id=%d (%s) op=%c res=%s", req->prod, req->db, req->op, resp->data);

	return 0;
}
