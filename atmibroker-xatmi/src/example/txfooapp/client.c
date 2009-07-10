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
#include "tx.h"

#include "userlogc.h"
#include "request.h"

extern int ora_access(test_req_t *req, test_req_t *resp);
extern int bdb_access(test_req_t *req, test_req_t *resp);
extern int bdb_test(const char * backingfile, const char * dbname, char ** rdata, int argc, char * argv[]);
extern int DISABLE_XA;

static enum DB_TYPE prod;
static char *emps[] = {"8000", "8001", "8002", "8003", "8004"};
static const char *dbfile1 = "db1"; /* backing file for database 1 */
static const char *dbfile2 = "db2"; /* backing file for database 2 */

static int is_begin(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_BEGIN || txtype == TX_TYPE_BEGIN_COMMIT || txtype == TX_TYPE_BEGIN_ABORT);
}
static int is_commit(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_COMMIT || txtype == TX_TYPE_BEGIN_COMMIT);
}
static int is_abort(enum TX_TYPE txtype) {
	return (txtype == TX_TYPE_ABORT || txtype == TX_TYPE_BEGIN_ABORT);
}
static int start_tx(enum TX_TYPE txtype) {
	if (is_begin(txtype)) {
		userlogc((char*) "- Starting Transaction");
		if (tx_begin() != TX_OK) {
			userlogc((char*) "TX ERROR - Could not begin transaction: ");
			return -1;
		}
	}

	return 0;
}
static int end_tx(enum TX_TYPE txtype) {
	int rv = 0;
	if (is_commit(txtype)) {
		userlogc((char*) "- Commiting transaction");
		if (tx_commit() != TX_OK)
			rv = -1;
	} else if (is_abort(txtype)) {
		userlogc((char*) "- Rolling back transaction");
		if (tx_rollback() != TX_OK)
			rv = -1;
	}

	if (rv != 0)
		userlogc((char*) "TX ERROR - Could not terminate transaction");

	return rv;
}

static int local_crud(const char *msg, char op, char *empno, enum TX_TYPE txtype)
{
	test_req_t req = {"db1", "8000", '1', prod, TX_TYPE_BEGIN_COMMIT, 0};
	test_req_t resp = {"db1", "", '1', prod, TX_TYPE_BEGIN_COMMIT, 0};

	if (msg) userlogc((char *) msg);

	(void) strcpy(req.data, empno);
	req.op = op;

	if (start_tx(txtype) == 0) {
		int rv;

		if (prod == ORACLE)
			rv = ora_access(&req, &resp);
		else if (prod == BDB)
			rv = bdb_access(&req, &resp);

		if (end_tx(txtype) == 0)
			return rv;
	}

	return -1;
}

static int send_req(const char * msg, const char *dbfile, const char *data, char op, enum TX_TYPE txtype, char **prbuf) {
	long rsz = sizeof (test_req_t);
	long callflags = 0L;
	test_req_t *req;
	test_req_t *resp;
	int rv = 0;

if (prod == BDB)	/* TODO remote */
	return local_crud(msg, op, (char *) data, txtype);

	if (start_tx(txtype) != 0)
		return -1;

	if (msg) userlogc((char *) msg);

	resp = (test_req_t *) tpalloc((char*) "X_C_TYPE", (char*) "dc_buf", sizeof (test_req_t));
	req = get_tbuf(data, dbfile, op, prod, txtype);

	if (tpcall("DBS", (char *) req, sizeof (test_req_t), (char **) &resp, &rsz, callflags) == -1) {
		userlogc((char*) "TP ERROR tperrno: %d", tperrno);
		rv = -1;
	}

	if (end_tx(txtype) != 0)
		rv = -1;

	tpfree((char *) req);
	tpfree((char *) resp);

	return rv;
}

static int count_records(const char *msg, char *key, int in_tx)
{
	test_req_t req = {"db1", "8000", '1', prod, TX_TYPE_BEGIN_COMMIT, 0};
	test_req_t resp = {"db1", "", '1', prod, TX_TYPE_BEGIN_COMMIT, 0};
	int cnt = -1;

	(void) strcpy(req.data, key);

	if (in_tx || start_tx(TX_TYPE_BEGIN) == 0) {
		if (msg) userlogc((char *) msg);
		if (prod == ORACLE)
			cnt = (ora_access(&req, &resp) == 0 ? atoi(resp.data) : -1);
		else if (prod == BDB)
			cnt = (bdb_access(&req, &resp) == 0 ? atoi(resp.data) : -1);
		else
			userlogc("product type not supported: %d", prod);

		userlogc("Record count: %d", cnt);

		if (in_tx || end_tx(TX_TYPE_COMMIT) == 0)
			return cnt;
	}

	return -1;
}

static int check_count(const char *msg, char *key, int in_tx, int expect) {
	int rcnt;

	if ((rcnt = count_records(msg, key, 0)) != expect) {
		userlogc("WRONG NUMBER OF RECORDS: rnt=%d", rcnt);
		printf("WRONG NUMBER OF RECORDS: rnt=%d", rcnt);
		return -1;
	}

	return 0;
}

static int run_test()
{
	int rv;

	/* start off with no records */
	if (count_records("COUNT RECORDS", emps[0], 0) > 0 &&
		(rv = send_req("DELETE AT SETUP", "", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0)) != 0)
		return rv;

	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 0)))
		return rv;

	/* ask the remote service to insert a record */
	if ((rv = send_req("REMOTE INSERT 1", "", emps[1], '0', TX_TYPE_BEGIN, 0)))
		return rv;
	/* ask the remote service to insert another record in the same transaction */
	if ((rv = send_req("REMOTE INSERT 2", "", emps[2], '0', TX_TYPE_NONE, 0)))
		return rv;

	/* insert a record */
	if ((rv = local_crud("LOCAL INSERT", '0', emps[3], TX_TYPE_COMMIT)))
		return rv;

	/* make sure the record count is 3 */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 3)))
		return -1;

	/* modify one of the records */
	if ((rv = send_req("REMOTE UPDATE", "", emps[1], '2', TX_TYPE_BEGIN_COMMIT, 0)))
		return rv;

	/* delete records starting from emps[0], */
	if ((rv = send_req("REMOTE DELETE", "", emps[0], '3', TX_TYPE_BEGIN_COMMIT, 0)))
		return rv;

	/* make sure the record count is back to zero again */
	if ((rv = check_count("COUNT RECORDS", emps[0], 0, 0)))
		return rv;

	return 0;
}

int main(int argc, char **argv)
{
	int rv = -1;

DISABLE_XA = 0;
	if (argc <= 1 || strcmp("bdb", argv[1]) == 0)
		prod = BDB;
	else if (strcmp("ora", argv[1]) == 0)
		prod = ORACLE;
	else
		fatal("Unsupported db - use ora or bdb\n");

	if (tx_open() != TX_OK)
		fatal("ERROR - Could not open transaction: ");

	rv = run_test();

	if (tx_close() != TX_OK) {
		userlogc((char*) "ERROR - Could not close transaction: ");
		rv = -1;
	}

	fprintf(stdout, "Test %s (%d)\n", (rv ? "failed" : "passed"), rv);

	return rv;
}
