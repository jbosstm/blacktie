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
#include <stdlib.h>
#include <string.h>

#include "xatmi.h"
#include "tx.h"

#include "userlogc.h"
#include "request.h"

static const char *dbfile1 = "db1"; /* backing file for database 1 */
static const char *dbfile2 = "db2"; /* backing file for database 2 */

static char * get_buf(const char *data, const char *dbfile, enum TX_TYPE txtype) {
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
static char * get_tbuf(const char *data, const char *dbfile, char op, enum TX_TYPE txtype) {
	test_req_t *req = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));

	strcpy(req->data, data);
	strcpy(req->db, dbfile);
	req->txtype = txtype;
	*(req->op) = op;

	return (char *) req;
}

static int is_begin(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_BEGIN || txtype == TX_TYPE_BEGIN_COMMIT || txtype == TX_TYPE_BEGIN_ABORT);
}
static int is_commit(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_COMMIT || txtype == TX_TYPE_BEGIN_COMMIT);
}
static int is_abort(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_ABORT || txtype == TX_TYPE_BEGIN_ABORT);
}

static int rw_record(const char *data, const char *dbfile, char op, enum TX_TYPE txtype, char **prbuf) {
	long rsz;
	int tpstatus;
	char *sbuf;
	char *rbuf = tpalloc("X_OCTET", 0, sizeof (test_req_t));
	long callflags = 0L;

	if (is_begin(txtype) && tx_begin() != TX_OK) {
		userlogc((char*) "ERROR - Could not begin transaction: ");
		return -1;
	}

	sbuf = get_tbuf(data, dbfile, op, txtype);

	// tpcall
	tpstatus = tpcall("DBS", sbuf, sizeof (test_req_t), (char **) &rbuf, &rsz, callflags);
	userlogc((char*) "tpcall status %d with output: %s", tpstatus, rbuf);

	if (prbuf)
		strcpy(*prbuf, rbuf);

	tpfree(sbuf);
	tpfree(rbuf);

	if (is_commit(txtype) && tx_commit() != TX_OK)
		userlogc((char*) "ERROR - Could not commit transaction: %s", sbuf);
	else if (is_abort(txtype) && tx_rollback() != TX_OK)
		userlogc((char*) "ERROR - Could not rollback transaction: %s", sbuf);

	return 0;
}

static void check_update(const char *db, char *key, char *val, int exists)
{
	char *rbuf = (char *) malloc(64);
	(void) rw_record(key, db, 'r', TX_TYPE_BEGIN_COMMIT, &rbuf);
	int found = (strcmp(rbuf, val) == 0);

	if (found != exists)
		userlogc((char*) "UPDATE ERROR - %s: %s %c= %s", key, rbuf, (exists == 0 ? '!' : '='), val);
	free(rbuf);
}

int main(int argc, char **argv)
{
	if (tx_open() != TX_OK) {
		userlogc((char*) "ERROR - Could not open transaction: ");
		return -1;
	}

#if 0
	check_update(dbfile1, "record1", "Amos", 1);
#else
	(void) rw_record("", dbfile1, 'd', TX_TYPE_BEGIN_COMMIT, NULL);
	(void) rw_record("", dbfile1, 'd', TX_TYPE_BEGIN_COMMIT, NULL);

	(void) rw_record("record1=Amos", dbfile1, 'w', TX_TYPE_BEGIN_COMMIT, NULL);
	check_update(dbfile1, "record1", "Amos", 1);

	(void) rw_record("record2=Michael", dbfile1, 'w', TX_TYPE_BEGIN_ABORT, NULL);
	check_update(dbfile1, "record2", "Michael", 0); /* record should not be updated */

	(void) rw_record("record3=Amos", dbfile1, 'w', TX_TYPE_BEGIN, NULL);
	(void) rw_record("record4=Michael", dbfile1, 'w', TX_TYPE_COMMIT, NULL);

	check_update(dbfile1, "record3", "Amos", 1);
	check_update(dbfile1, "record4", "Michael", 1);
#endif

	if (tx_close() != TX_OK) {
		userlogc((char*) "ERROR - Could not close transaction: ");
	}

	return 0;
}
